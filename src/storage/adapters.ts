import { PROVIDER_KIND_META } from '../lib/constants';
import { SI_MB } from '../lib/format';
import {
  StorageError,
  type ConnectionReport,
  type DeveloperSettings,
  type ProviderKind,
  type ProviderMount,
  type StorageProviderAdapter,
} from '../types/domain';
import type { VaultRepository } from './repository';

const MIB = 1024 * 1024;

function abortableSleep(
  ms: number,
  signal?: AbortSignal,
  onTick?: (fraction: number) => void
): Promise<void> {
  return new Promise((resolve, reject) => {
    if (signal?.aborted) {
      reject(new StorageError('ABORTED', 'Operation aborted.', false));
      return;
    }

    const start = performance.now();
    let tickTimer: ReturnType<typeof setInterval> | null = null;

    if (onTick) {
      tickTimer = setInterval(() => {
        const elapsed = performance.now() - start;
        const frac = Math.min(0.99, elapsed / Math.max(1, ms));
        onTick(frac);
      }, 100);
    }

    const timeout = setTimeout(() => {
      if (tickTimer) clearInterval(tickTimer);
      signal?.removeEventListener('abort', onAbort);
      onTick?.(1);
      resolve();
    }, ms);

    const onAbort = () => {
      clearTimeout(timeout);
      if (tickTimer) clearInterval(tickTimer);
      reject(new StorageError('ABORTED', 'Operation aborted.', false));
    };

    signal?.addEventListener('abort', onAbort, { once: true });
  });
}

// Local IndexedDB Adapter (kind = 'local', partSizeBytes = 1 MiB)
export class IndexedDBStorageAdapter implements StorageProviderAdapter {
  public readonly kind: ProviderKind = 'local';
  public readonly partSizeBytes = 1 * MIB;
  private progressCallback?: (uploadId: string, partNumber: number, bytesInPart: number) => void;

  constructor(private repo: VaultRepository) {}

  public setPartProgressListener(
    cb: (uploadId: string, partNumber: number, bytesInPart: number) => void
  ): void {
    this.progressCallback = cb;
  }

  public async testConnection(_m: ProviderMount, signal?: AbortSignal): Promise<ConnectionReport> {
    const start = performance.now();
    await abortableSleep(45, signal);
    const latencyMs = Math.max(2, Math.round(performance.now() - start));
    return {
      ok: true,
      latencyMs,
      canList: true,
      canRead: true,
      canWrite: !this.repo.usingMemoryFallback,
      message: 'Local Browser IndexedDB store connected.',
    };
  }

  public async createMultipartUpload(
    m: ProviderMount,
    _key: string,
    _contentType: string
  ): Promise<{ uploadId: string }> {
    if (m.status === 'disconnected') {
      throw new StorageError('DISCONNECTED', `${m.name} is disconnected.`, false);
    }
    return { uploadId: `upl-local-${crypto.randomUUID()}` };
  }

  public async uploadPart(
    m: ProviderMount,
    _key: string,
    uploadId: string,
    partNumber: number,
    data: Blob,
    signal: AbortSignal
  ): Promise<{ etag: string }> {
    if (m.status === 'disconnected') {
      throw new StorageError('DISCONNECTED', `${m.name} is disconnected.`, false);
    }
    const durationMs = Math.max(220, Math.min(750, Math.round((data.size / (18 * SI_MB)) * 1000)));
    await abortableSleep(durationMs, signal, (frac) => {
      this.progressCallback?.(uploadId, partNumber, Math.round(data.size * frac));
    });
    return { etag: `"etag-local-${partNumber}-${data.size}"` };
  }

  public async completeMultipartUpload(
    _m: ProviderMount,
    _key: string,
    _uploadId: string,
    _parts: { partNumber: number; etag: string }[],
    _source: Blob
  ): Promise<void> {
    // Blob persistence is committed atomically in vaultRepository.commitCompletedUpload
  }

  public async abortMultipartUpload(
    _m: ProviderMount,
    _key: string,
    _uploadId: string
  ): Promise<void> {}

  public async getObject(m: ProviderMount, key: string): Promise<Blob> {
    const blob = await this.repo.getBlob(m.id, key);
    if (!blob) {
      throw new StorageError('NOT_FOUND', 'Object bytes not found in local vault.', false);
    }
    return blob;
  }

  public async deleteObject(m: ProviderMount, key: string): Promise<void> {
    await this.repo.deleteBlob(m.id, key);
  }

  public async getUsage(m: ProviderMount): Promise<{ usedBytes: number; objectCount: number }> {
    const nodes = await this.repo.getAllNodes();
    let usedBytes = 0;
    let objectCount = 0;
    for (const n of nodes) {
      if (n.type === 'file' && n.providerId === m.id) {
        usedBytes += n.sizeBytes;
        objectCount += 1;
      }
    }
    return { usedBytes, objectCount };
  }
}

// Universal Simulated Cloud Adapter supporting all 21 simulated cloud provider kinds
export class SimulatedCloudAdapter implements StorageProviderAdapter {
  public readonly kind: ProviderKind;
  public readonly partSizeBytes: number;
  protected readonly baseLatencyMs: number;
  protected readonly minBandwidthMBps: number;
  protected readonly maxBandwidthMBps: number;

  private progressCallback?: (uploadId: string, partNumber: number, bytesInPart: number) => void;

  constructor(
    kind: ProviderKind,
    protected repo: VaultRepository,
    protected getDevSettings: () => DeveloperSettings
  ) {
    this.kind = kind;
    const meta = PROVIDER_KIND_META[kind];
    this.partSizeBytes = meta.partSizeMiB * MIB;
    this.baseLatencyMs = meta.baseLatencyMs;
    this.minBandwidthMBps = meta.minBwMBps;
    this.maxBandwidthMBps = meta.maxBwMBps;
  }

  public setPartProgressListener(
    cb: (uploadId: string, partNumber: number, bytesInPart: number) => void
  ): void {
    this.progressCallback = cb;
  }

  protected computeJitteredLatency(): number {
    const jitter = 0.7 + Math.random() * 0.6;
    return Math.max(10, Math.round(this.baseLatencyMs * jitter));
  }

  protected computeSimulatedBandwidthBytesPerSec(): number {
    const mbps =
      this.minBandwidthMBps +
      Math.random() * (this.maxBandwidthMBps - this.minBandwidthMBps);
    return mbps * SI_MB;
  }

  protected checkMountAndDevFaults(m: ProviderMount): void {
    if (m.status === 'disconnected') {
      throw new StorageError(
        'DISCONNECTED',
        `Mount "${m.name}" is currently disconnected.`,
        false
      );
    }
    const dev = this.getDevSettings();
    if (dev.offline) {
      throw new StorageError(
        'NETWORK',
        `Simulated offline mode is active — cannot reach ${m.name}.`,
        true
      );
    }
    if (dev.failureRate > 0) {
      const roll = Math.random() * 100;
      if (roll < dev.failureRate) {
        throw new StorageError(
          'NETWORK',
          `Simulated transient network fault (${dev.failureRate}% failure rate) on ${m.name}.`,
          true
        );
      }
    }
  }

  public async testConnection(m: ProviderMount, signal?: AbortSignal): Promise<ConnectionReport> {
    const dev = this.getDevSettings();
    const latencyMs = this.computeJitteredLatency();
    await abortableSleep(latencyMs, signal);

    if (dev.offline) {
      throw new StorageError(
        'NETWORK',
        `Simulated network offline while probing ${m.name}.`,
        true
      );
    }

    return {
      ok: true,
      latencyMs,
      canList: true,
      canRead: true,
      canWrite: true,
      message: `Simulated ${PROVIDER_KIND_META[this.kind].shortLabel} handshake verified (${latencyMs} ms).`,
    };
  }

  public async createMultipartUpload(
    m: ProviderMount,
    _key: string,
    _contentType: string
  ): Promise<{ uploadId: string }> {
    this.checkMountAndDevFaults(m);
    await abortableSleep(this.computeJitteredLatency());
    return { uploadId: `upl-${this.kind}-${crypto.randomUUID()}` };
  }

  public async uploadPart(
    m: ProviderMount,
    _key: string,
    uploadId: string,
    partNumber: number,
    data: Blob,
    signal: AbortSignal
  ): Promise<{ etag: string }> {
    this.checkMountAndDevFaults(m);

    const bw = this.computeSimulatedBandwidthBytesPerSec();
    const transferMs = Math.max(
      250,
      Math.round(this.computeJitteredLatency() + (data.size / bw) * 1000)
    );

    await abortableSleep(transferMs, signal, (frac) => {
      if (this.getDevSettings().offline) {
        return;
      }
      this.progressCallback?.(uploadId, partNumber, Math.round(data.size * frac));
    });

    this.checkMountAndDevFaults(m);

    return { etag: `"etag-${this.kind}-${partNumber}-${data.size}"` };
  }

  public async completeMultipartUpload(
    m: ProviderMount,
    _key: string,
    _uploadId: string,
    _parts: { partNumber: number; etag: string }[],
    _source: Blob
  ): Promise<void> {
    this.checkMountAndDevFaults(m);
    await abortableSleep(this.computeJitteredLatency());
    // Blob persistence is committed atomically in vaultRepository.commitCompletedUpload
  }

  public async abortMultipartUpload(
    _m: ProviderMount,
    _key: string,
    _uploadId: string
  ): Promise<void> {}

  public async getObject(m: ProviderMount, key: string): Promise<Blob> {
    if (m.status === 'disconnected') {
      throw new StorageError('DISCONNECTED', `Mount "${m.name}" is disconnected.`, false);
    }
    if (this.getDevSettings().offline) {
      throw new StorageError('NETWORK', 'Simulated offline mode is active.', true);
    }
    await abortableSleep(Math.min(120, this.computeJitteredLatency()));
    const blob = await this.repo.getBlob(m.id, key);
    if (!blob) {
      throw new StorageError('NOT_FOUND', `Object not found on ${m.name}.`, false);
    }
    return blob;
  }

  public async deleteObject(m: ProviderMount, key: string): Promise<void> {
    await abortableSleep(Math.min(90, this.computeJitteredLatency()));
    await this.repo.deleteBlob(m.id, key);
  }

  public async getUsage(m: ProviderMount): Promise<{ usedBytes: number; objectCount: number }> {
    const nodes = await this.repo.getAllNodes();
    let usedBytes = 0;
    let objectCount = 0;
    for (const n of nodes) {
      if (n.type === 'file' && n.providerId === m.id) {
        usedBytes += n.sizeBytes;
        objectCount += 1;
      }
    }
    return { usedBytes, objectCount };
  }
}
