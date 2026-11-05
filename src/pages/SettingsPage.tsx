import {
  AlertTriangle,
  Check,
  Copy,
  Database,
  KeyRound,
  LogOut,
  RotateCcw,
  Sliders,
  Trash2,
  User,
} from 'lucide-react';
import React, { useEffect, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { formatBytes, formatFullTimestamp } from '../lib/format';
import { useVaultStore } from '../store/useVaultStore';
import type { FailureRatePercent } from '../types/domain';

export function SettingsPage() {
  const navigate = useNavigate();
  const currentAccount = useVaultStore((s) => s.currentAccount);
  const settings = useVaultStore((s) => s.settings);
  const usingMemoryFallback = useVaultStore((s) => s.usingMemoryFallback);
  const updateProfile = useVaultStore((s) => s.updateProfile);
  const updateDeveloperSettings = useVaultStore((s) => s.updateDeveloperSettings);
  const generateDemoToken = useVaultStore((s) => s.generateDemoToken);
  const revokeDemoToken = useVaultStore((s) => s.revokeDemoToken);
  const resetSampleWorkspace = useVaultStore((s) => s.resetSampleWorkspace);
  const signOut = useVaultStore((s) => s.signOut);
  const addToast = useVaultStore((s) => s.addToast);

  const [displayName, setDisplayName] = useState(currentAccount?.displayName ?? '');
  const [tokenName, setTokenName] = useState('');
  const [confirmResetOpen, setConfirmResetOpen] = useState(false);
  const [resetting, setResetting] = useState(false);
  const [copiedTokenId, setCopiedTokenId] = useState<string | null>(null);

  const handleCopyToken = async (id: string, text: string) => {
    let success = false;
    if (navigator.clipboard?.writeText) {
      try {
        await navigator.clipboard.writeText(text);
        success = true;
      } catch {
        // Fallback below
      }
    }
    if (!success) {
      try {
        const textArea = document.createElement('textarea');
        textArea.value = text;
        textArea.style.position = 'fixed';
        textArea.style.opacity = '0';
        document.body.appendChild(textArea);
        textArea.select();
        success = document.execCommand('copy');
        document.body.removeChild(textArea);
      } catch {
        success = false;
      }
    }

    if (success) {
      setCopiedTokenId(id);
      setTimeout(() => setCopiedTokenId((curr) => (curr === id ? null : curr)), 2000);
      addToast({ title: 'Copied demo token to clipboard', variant: 'success' });
    } else {
      addToast({ title: 'Failed to copy token to clipboard', variant: 'error' });
    }
  };

  // Browser Storage Estimate & Persisted status (§7)
  const [storageEstimate, setStorageEstimate] = useState<{
    usage: number;
    quota: number;
    persisted: boolean;
  } | null>(null);

  useEffect(() => {
    setDisplayName(currentAccount?.displayName ?? '');
  }, [currentAccount]);

  useEffect(() => {
    let cancelled = false;
    (async () => {
      try {
        const est = (await navigator.storage?.estimate?.()) ?? { usage: 0, quota: 0 };
        const persisted = (await navigator.storage?.persisted?.()) ?? false;
        if (!cancelled) {
          setStorageEstimate({
            usage: est.usage ?? 0,
            quota: est.quota ?? 0,
            persisted,
          });
        }
      } catch {
        if (!cancelled) {
          setStorageEstimate({ usage: 0, quota: 0, persisted: false });
        }
      }
    })();
    return () => {
      cancelled = true;
    };
  }, [settings]);

  const handleSaveProfile = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      await updateProfile(displayName);
      addToast({ title: 'Updated local demo profile', variant: 'success' });
    } catch (err) {
      addToast({
        title: 'Could not update profile',
        description: err instanceof Error ? err.message : undefined,
        variant: 'error',
      });
    }
  };

  const handleCreateToken = async (e: React.FormEvent) => {
    e.preventDefault();
    const val = await generateDemoToken(tokenName);
    setTokenName('');
    addToast({
      title: 'Generated demo API token',
      description: `Token ${val.slice(0, 14)}… created locally.`,
      variant: 'success',
    });
  };

  return (
    <div className="p-4 lg:p-6 space-y-6 max-w-4xl w-full mx-auto">
      <div>
        <h1 className="text-xl font-bold text-[#F3F4F6]">Workspace Settings</h1>
        <p className="text-xs text-[#8A94A6] mt-0.5">
          Manage your local demo profile, demo API tokens, IndexedDB browser storage status, and adapter fault simulation.
        </p>
      </div>

      {/* 1. Profile Settings (§7) */}
      <section className="rounded-lg bg-[#131620] border border-[#1E2230] p-5 space-y-4">
        <div className="flex items-center gap-2">
          <User className="w-4 h-4 text-[#818CF8]" />
          <h2 className="text-[15px] font-bold text-[#F3F4F6]">Local Demo Profile</h2>
        </div>

        <form onSubmit={handleSaveProfile} className="grid grid-cols-1 sm:grid-cols-2 gap-4">
          <div>
            <label htmlFor="profile-name" className="block text-xs font-medium text-[#8A94A6] mb-1">
              Display Name
            </label>
            <input
              id="profile-name"
              type="text"
              value={displayName}
              onChange={(e) => setDisplayName(e.target.value)}
              className="w-full px-3 py-2 rounded-md bg-[#0E1017] border border-[#1E2230] text-[13px] text-[#F3F4F6]"
            />
          </div>

          <div>
            <label htmlFor="profile-email" className="block text-xs font-medium text-[#8A94A6] mb-1">
              Local Account Email
            </label>
            <input
              id="profile-email"
              type="email"
              disabled
              value={currentAccount?.email ?? ''}
              className="w-full px-3 py-2 rounded-md bg-[#090A0F] border border-[#1E2230] text-[13px] text-[#8A94A6] font-mono cursor-not-allowed"
            />
          </div>

          <div className="sm:col-span-2 flex justify-end">
            <button
              type="submit"
              className="px-4 py-1.5 rounded-md bg-[#4F46E5] hover:bg-[#4338CA] text-white text-xs font-medium"
            >
              Save Profile
            </button>
          </div>
        </form>
      </section>

      {/* 2. Developer Simulation Panel (§4, §7: failure rate 0/10/50% and Offline toggle) */}
      <section className="rounded-lg bg-[#131620] border border-[#1E2230] p-5 space-y-4">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-2">
            <Sliders className="w-4 h-4 text-[#818CF8]" />
            <h2 className="text-[15px] font-bold text-[#F3F4F6]">
              Developer Panel (Simulated Adapter Fault Injection)
            </h2>
          </div>
          <span className="text-[11px] font-mono text-[#8A94A6] bg-[#0E1017] border border-[#1E2230] px-2 py-0.5 rounded">
            Applies to S3, R2, GCS, WebDAV
          </span>
        </div>

        <p className="text-xs text-[#8A94A6]">
          Test multipart upload retries (<code className="font-mono text-[#F3F4F6]">500ms · 2^n</code> backoff) and error states by injecting transient network faults or toggling simulated offline mode.
        </p>

        <div className="grid grid-cols-1 sm:grid-cols-2 gap-4 pt-1">
          {/* Failure Rate Selector (0 / 10 / 50 %) */}
          <div className="p-3.5 rounded-md bg-[#0E1017] border border-[#1E2230] space-y-2">
            <span className="block text-xs font-medium text-[#F3F4F6]">
              Simulated Part Failure Rate
            </span>
            <div role="group" aria-label="Simulated failure rate" className="grid grid-cols-3 gap-2">
              {([0, 10, 50] as FailureRatePercent[]).map((rate) => {
                const active = settings.devSettings.failureRate === rate;
                return (
                  <button
                    key={rate}
                    type="button"
                    onClick={() => {
                      updateDeveloperSettings({ failureRate: rate });
                      addToast({
                        title: `Simulated failure rate set to ${rate}%`,
                        variant: rate > 0 ? 'warning' : 'default',
                      });
                    }}
                    className={`py-1.5 rounded text-xs font-mono font-medium border ${
                      active
                        ? 'bg-[#4F46E5] border-[#818CF8] text-white'
                        : 'bg-[#131620] border-[#1E2230] text-[#8A94A6] hover:text-[#F3F4F6]'
                    }`}
                  >
                    {rate}%
                  </button>
                );
              })}
            </div>
          </div>

          {/* Offline Mode Toggle */}
          <div className="p-3.5 rounded-md bg-[#0E1017] border border-[#1E2230] flex items-center justify-between gap-3">
            <div>
              <p className="text-xs font-medium text-[#F3F4F6]">Simulated Offline Mode</p>
              <p className="text-[11px] text-[#8A94A6] mt-0.5">
                Throws retryable <code className="font-mono">NETWORK</code> errors on cloud adapters
              </p>
            </div>
            <button
              type="button"
              role="switch"
              aria-checked={settings.devSettings.offline}
              onClick={() => {
                const next = !settings.devSettings.offline;
                updateDeveloperSettings({ offline: next });
                addToast({
                  title: next
                    ? 'Simulated Offline Mode enabled'
                    : 'Simulated Offline Mode disabled',
                  variant: next ? 'warning' : 'success',
                });
              }}
              className={`px-3 py-1.5 rounded text-xs font-mono font-medium border ${
                settings.devSettings.offline
                  ? 'bg-[#F43F5E] border-[#F43F5E] text-white'
                  : 'bg-[#131620] border-[#1E2230] text-[#8A94A6] hover:text-[#F3F4F6]'
              }`}
            >
              {settings.devSettings.offline ? 'OFFLINE ON' : 'OFFLINE OFF'}
            </button>
          </div>
        </div>
      </section>

      {/* 3. Demo API Tokens (§7: generate random, copy, revoke; labelled demo) */}
      <section className="rounded-lg bg-[#131620] border border-[#1E2230] p-5 space-y-4">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-2">
            <KeyRound className="w-4 h-4 text-[#818CF8]" />
            <h2 className="text-[15px] font-bold text-[#F3F4F6]">Demo API Tokens</h2>
          </div>
          <span className="text-xs font-mono text-[#8A94A6]">Demo only — local browser state</span>
        </div>

        <form onSubmit={handleCreateToken} className="flex items-center gap-2">
          <input
            type="text"
            value={tokenName}
            onChange={(e) => setTokenName(e.target.value)}
            placeholder="Token label (e.g. Rclone Local Script Demo)"
            aria-label="Demo token name"
            className="flex-1 px-3 py-2 rounded-md bg-[#0E1017] border border-[#1E2230] text-[13px] text-[#F3F4F6] placeholder-[#8A94A6]"
          />
          <button
            type="submit"
            className="px-3.5 py-2 rounded-md bg-[#4F46E5] hover:bg-[#4338CA] text-white text-xs font-medium whitespace-nowrap"
          >
            Generate Demo Token
          </button>
        </form>

        <div className="divide-y divide-[#1E2230] border border-[#1E2230] rounded-md bg-[#0E1017]">
          {settings.demoTokens.length === 0 ? (
            <p className="p-4 text-xs text-[#8A94A6] text-center">No demo API tokens active.</p>
          ) : (
            settings.demoTokens.map((tok) => (
              <div
                key={tok.id}
                className="px-4 py-3 flex items-center justify-between gap-3 text-xs"
              >
                <div className="min-w-0">
                  <div className="flex items-center gap-2">
                    <span className="font-medium text-[#F3F4F6]">{tok.name}</span>
                    <span className="px-1.5 py-0.5 text-[10px] font-mono bg-[#131620] border border-[#1E2230] text-[#8A94A6] rounded">
                      Demo
                    </span>
                  </div>
                  <p className="font-mono text-[#8A94A6] mt-0.5 truncate tabular-nums">
                    {tok.tokenValue} · Created {formatFullTimestamp(tok.createdAt)}
                  </p>
                </div>

                <div className="flex items-center gap-2 shrink-0">
                  <button
                    type="button"
                    onClick={() => handleCopyToken(tok.id, tok.tokenValue)}
                    className="inline-flex items-center gap-1 px-2.5 py-1 rounded bg-[#131620] hover:bg-[#1E2230] border border-[#1E2230] text-[#F3F4F6]"
                  >
                    {copiedTokenId === tok.id ? (
                      <>
                        <Check className="w-3 h-3 text-[#10B981]" />
                        <span className="text-[#10B981]">Copied</span>
                      </>
                    ) : (
                      <>
                        <Copy className="w-3 h-3" />
                        <span>Copy</span>
                      </>
                    )}
                  </button>
                  <button
                    type="button"
                    onClick={async () => {
                      await revokeDemoToken(tok.id);
                      addToast({ title: `Revoked demo token "${tok.name}"`, variant: 'default' });
                    }}
                    className="inline-flex items-center gap-1 px-2.5 py-1 rounded bg-[#131620] hover:bg-[#F43F5E]/20 border border-[#1E2230] text-[#F43F5E]"
                  >
                    <Trash2 className="w-3 h-3" />
                    <span>Revoke</span>
                  </button>
                </div>
              </div>
            ))
          )}
        </div>
      </section>

      {/* 4. Browser Storage Status & Workspace Reset / Sign Out (§3, §7) */}
      <section className="rounded-lg bg-[#131620] border border-[#1E2230] p-5 space-y-4">
        <div className="flex items-center gap-2">
          <Database className="w-4 h-4 text-[#818CF8]" />
          <h2 className="text-[15px] font-bold text-[#F3F4F6]">
            Browser IndexedDB Storage Status
          </h2>
        </div>

        <div className="grid grid-cols-1 sm:grid-cols-3 gap-3 font-mono text-xs tabular-nums">
          <div className="p-3 rounded bg-[#0E1017] border border-[#1E2230]">
            <p className="text-[#8A94A6] font-sans">Database Engine</p>
            <p className="text-[#F3F4F6] font-bold mt-1">
              {usingMemoryFallback ? 'In-Memory Fallback' : 'IndexedDB (televault_db)'}
            </p>
          </div>
          <div className="p-3 rounded bg-[#0E1017] border border-[#1E2230]">
            <p className="text-[#8A94A6] font-sans">Browser Estimate (Usage / Quota)</p>
            <p className="text-[#F3F4F6] font-bold mt-1">
              {storageEstimate
                ? `${formatBytes(storageEstimate.usage)} / ${formatBytes(storageEstimate.quota)}`
                : 'Calculating…'}
            </p>
          </div>
          <div className="p-3 rounded bg-[#0E1017] border border-[#1E2230]">
            <p className="text-[#8A94A6] font-sans">Persistent Storage Granted</p>
            <p className="text-[#F3F4F6] font-bold mt-1">
              {storageEstimate ? (storageEstimate.persisted ? 'Yes' : 'No (Best-effort)') : '—'}
            </p>
          </div>
        </div>

        <div className="pt-3 border-t border-[#1E2230] flex flex-wrap items-center justify-between gap-3">
          <button
            type="button"
            onClick={() => setConfirmResetOpen(true)}
            className="inline-flex items-center gap-2 px-3.5 py-2 rounded-md bg-[#F59E0B]/15 hover:bg-[#F59E0B]/25 border border-[#F59E0B]/40 text-xs font-medium text-[#F59E0B]"
          >
            <RotateCcw className="w-3.5 h-3.5" />
            <span>Reset to sample workspace</span>
          </button>

          <button
            type="button"
            onClick={async () => {
              await signOut();
              navigate('/auth');
            }}
            className="inline-flex items-center gap-2 px-3.5 py-2 rounded-md bg-[#0E1017] hover:bg-[#F43F5E]/20 border border-[#1E2230] text-xs font-medium text-[#F43F5E]"
          >
            <LogOut className="w-3.5 h-3.5" />
            <span>Sign out</span>
          </button>
        </div>
      </section>

      {/* Reset Confirmation Modal (§3) */}
      {confirmResetOpen && (
        <div
          className="fixed inset-0 z-50 flex items-center justify-center bg-black/75 p-4"
          role="dialog"
          aria-modal="true"
          aria-labelledby="reset-modal-title"
          onKeyDown={(e) => {
            if (e.key === 'Escape') setConfirmResetOpen(false);
          }}
        >
          <div className="w-full max-w-md rounded-lg bg-[#131620] border border-[#1E2230] p-5 space-y-4">
            <div className="flex items-start gap-3">
              <AlertTriangle className="w-5 h-5 text-[#F59E0B] shrink-0 mt-0.5" />
              <div>
                <h2 id="reset-modal-title" className="text-[15px] font-bold text-[#F3F4F6]">
                  Reset to deterministic sample workspace?
                </h2>
                <p className="text-xs text-[#8A94A6] mt-1 leading-relaxed">
                  This clears all uploaded files, custom folders, mount changes, and activity events in IndexedDB, preserving your local account and re-running the deterministic seed (<code className="font-mono">mulberry32(42)</code>).
                </p>
              </div>
            </div>

            <div className="flex items-center justify-end gap-2 pt-2">
              <button
                type="button"
                disabled={resetting}
                onClick={() => setConfirmResetOpen(false)}
                className="px-3.5 py-1.5 rounded-md border border-[#1E2230] text-xs font-medium text-[#8A94A6] hover:text-[#F3F4F6]"
              >
                Cancel
              </button>
              <button
                type="button"
                disabled={resetting}
                onClick={async () => {
                  setResetting(true);
                  await resetSampleWorkspace();
                  setResetting(false);
                  setConfirmResetOpen(false);
                  addToast({
                    title: 'Workspace reset to initial sample data',
                    variant: 'success',
                  });
                }}
                className="px-4 py-1.5 rounded-md bg-[#F59E0B] hover:bg-[#D97706] text-black text-xs font-bold disabled:opacity-50"
              >
                {resetting ? 'Resetting…' : 'Reset Workspace'}
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
