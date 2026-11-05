import type { FileCategory, ProviderKind } from '../types/domain';

export const APP_NAME = 'TeleVault';

export const PROTOTYPE_BADGE_TEXT = 'Prototype · simulated connectors · data stays in this browser';

export const DEMO_ACCOUNT_EMAIL = 'alex.rivera@televault.demo';
export const DEMO_ACCOUNT_NAME = 'Alex Rivera';
export const DEMO_ACCOUNT_DEFAULT_PASSWORD = 'demo-vault-password';

export const MAX_UPLOAD_SIZE_BYTES = 200 * 1_000_000; // 200 MB (SI)
export const MAX_SHA256_SIZE_BYTES = 64 * 1_000_000;  // 64 MB (SI)
export const MAX_TEXT_PREVIEW_BYTES = 1 * 1_000_000;  // 1 MB (SI)
export const TRASH_RETENTION_MS = 30 * 24 * 60 * 60 * 1000; // 30 days

export interface ProviderMetaInfo {
  label: string;
  shortLabel: string;
  category: 'Consumer & Personal Cloud' | 'Object & Infrastructure Cloud' | 'Self-Hosted & Protocol' | 'Built-In Browser';
  isSimulated: boolean;
  defaultZeroEgress: boolean;
  badgeColor: string;
  defaultEndpoint: string;
  defaultRegion: string;
  defaultAllocatedGB: number;
  baseLatencyMs: number;
  minBwMBps: number;
  maxBwMBps: number;
  partSizeMiB: number;
  description: string;
}

export const PROVIDER_KIND_META: Record<ProviderKind, ProviderMetaInfo> = {
  local: {
    label: 'IndexedDB Local Vault',
    shortLabel: 'Local Vault',
    category: 'Built-In Browser',
    isSimulated: false,
    defaultZeroEgress: true,
    badgeColor: '#818CF8',
    defaultEndpoint: 'idb://televault_db',
    defaultRegion: 'local-browser',
    defaultAllocatedGB: 10,
    baseLatencyMs: 4,
    minBwMBps: 25,
    maxBwMBps: 60,
    partSizeMiB: 1,
    description: 'Browser-native IndexedDB blob store in this profile',
  },
  mega: {
    label: 'MEGA Cloud Drive',
    shortLabel: 'MEGA',
    category: 'Consumer & Personal Cloud',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#F43F5E',
    defaultEndpoint: 'https://g.api.mega.co.nz',
    defaultRegion: 'eu-central-lux',
    defaultAllocatedGB: 2000,
    baseLatencyMs: 68,
    minBwMBps: 14,
    maxBwMBps: 32,
    partSizeMiB: 4,
    description: 'Simulated MEGA high-capacity cloud drive mount',
  },
  terabox: {
    label: 'TeraBox Cloud Storage',
    shortLabel: 'TeraBox',
    category: 'Consumer & Personal Cloud',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#38BDF8',
    defaultEndpoint: 'https://www.terabox.com/rest/2.0/pcs',
    defaultRegion: 'ap-northeast-1',
    defaultAllocatedGB: 1024,
    baseLatencyMs: 115,
    minBwMBps: 8,
    maxBwMBps: 22,
    partSizeMiB: 4,
    description: 'Simulated 1 TB+ TeraBox personal cloud vault mount',
  },
  gdrive: {
    label: 'Google Drive Workspace',
    shortLabel: 'Google Drive',
    category: 'Consumer & Personal Cloud',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#10B981',
    defaultEndpoint: 'https://www.googleapis.com/drive/v3',
    defaultRegion: 'us-multi',
    defaultAllocatedGB: 2000,
    baseLatencyMs: 72,
    minBwMBps: 15,
    maxBwMBps: 34,
    partSizeMiB: 8,
    description: 'Simulated Google Drive workspace file stream mount',
  },
  dropbox: {
    label: 'Dropbox Cloud',
    shortLabel: 'Dropbox',
    category: 'Consumer & Personal Cloud',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#60A5FA',
    defaultEndpoint: 'https://content.dropboxapi.com/2',
    defaultRegion: 'us-west',
    defaultAllocatedGB: 2000,
    baseLatencyMs: 76,
    minBwMBps: 14,
    maxBwMBps: 30,
    partSizeMiB: 8,
    description: 'Simulated Dropbox block-synced team folder mount',
  },
  onedrive: {
    label: 'Microsoft OneDrive',
    shortLabel: 'OneDrive',
    category: 'Consumer & Personal Cloud',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#3B82F6',
    defaultEndpoint: 'https://graph.microsoft.com/v1.0/me/drive',
    defaultRegion: 'global-msft',
    defaultAllocatedGB: 1000,
    baseLatencyMs: 84,
    minBwMBps: 12,
    maxBwMBps: 28,
    partSizeMiB: 8,
    description: 'Simulated OneDrive Microsoft Graph storage mount',
  },
  box: {
    label: 'Box Enterprise Cloud',
    shortLabel: 'Box',
    category: 'Consumer & Personal Cloud',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#38BDF8',
    defaultEndpoint: 'https://upload.box.com/api/2.0',
    defaultRegion: 'us-box',
    defaultAllocatedGB: 1000,
    baseLatencyMs: 88,
    minBwMBps: 11,
    maxBwMBps: 26,
    partSizeMiB: 8,
    description: 'Simulated Box content cloud storage mount',
  },
  pcloud: {
    label: 'pCloud Swiss Vault',
    shortLabel: 'pCloud',
    category: 'Consumer & Personal Cloud',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#2DD4BF',
    defaultEndpoint: 'https://eapi.pcloud.com',
    defaultRegion: 'eu-zurich',
    defaultAllocatedGB: 2000,
    baseLatencyMs: 74,
    minBwMBps: 14,
    maxBwMBps: 30,
    partSizeMiB: 4,
    description: 'Simulated pCloud European media drive mount',
  },
  icloud: {
    label: 'Apple iCloud Drive',
    shortLabel: 'iCloud Drive',
    category: 'Consumer & Personal Cloud',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#93C5FD',
    defaultEndpoint: 'https://ckdatabasews.icloud.com',
    defaultRegion: 'us-cupertino',
    defaultAllocatedGB: 2000,
    baseLatencyMs: 82,
    minBwMBps: 12,
    maxBwMBps: 27,
    partSizeMiB: 4,
    description: 'Simulated iCloud CloudKit container mount',
  },
  proton: {
    label: 'Proton Drive',
    shortLabel: 'Proton Drive',
    category: 'Consumer & Personal Cloud',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#A78BFA',
    defaultEndpoint: 'https://drive.proton.me/api',
    defaultRegion: 'ch-geneva',
    defaultAllocatedGB: 500,
    baseLatencyMs: 92,
    minBwMBps: 10,
    maxBwMBps: 24,
    partSizeMiB: 4,
    description: 'Simulated Proton Swiss privacy drive mount',
  },
  sync: {
    label: 'Sync.com Vault',
    shortLabel: 'Sync.com',
    category: 'Consumer & Personal Cloud',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#34D399',
    defaultEndpoint: 'https://api.sync.com/v2',
    defaultRegion: 'ca-central-1',
    defaultAllocatedGB: 2000,
    baseLatencyMs: 95,
    minBwMBps: 10,
    maxBwMBps: 22,
    partSizeMiB: 4,
    description: 'Simulated Sync.com Canadian cloud storage mount',
  },
  kdrive: {
    label: 'Infomaniak kDrive',
    shortLabel: 'kDrive',
    category: 'Consumer & Personal Cloud',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#60A5FA',
    defaultEndpoint: 'https://api.infomaniak.com/2/drive',
    defaultRegion: 'ch-winterthur',
    defaultAllocatedGB: 3000,
    baseLatencyMs: 79,
    minBwMBps: 13,
    maxBwMBps: 29,
    partSizeMiB: 4,
    description: 'Simulated Infomaniak kDrive collaborative cloud mount',
  },
  s3: {
    label: 'Amazon S3',
    shortLabel: 'AWS S3',
    category: 'Object & Infrastructure Cloud',
    isSimulated: true,
    defaultZeroEgress: false,
    badgeColor: '#F59E0B',
    defaultEndpoint: 'https://s3.us-east-1.amazonaws.com',
    defaultRegion: 'us-east-1',
    defaultAllocatedGB: 500,
    baseLatencyMs: 80,
    minBwMBps: 12,
    maxBwMBps: 30,
    partSizeMiB: 8,
    description: 'Simulated S3 standard object bucket',
  },
  r2: {
    label: 'Cloudflare R2',
    shortLabel: 'Cloudflare R2',
    category: 'Object & Infrastructure Cloud',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#FB923C',
    defaultEndpoint: 'https://88f19c2a.r2.cloudflarestorage.com',
    defaultRegion: 'wnam',
    defaultAllocatedGB: 1000,
    baseLatencyMs: 60,
    minBwMBps: 15,
    maxBwMBps: 35,
    partSizeMiB: 8,
    description: 'Simulated zero-egress edge object storage',
  },
  gcs: {
    label: 'Google Cloud Storage',
    shortLabel: 'Google Cloud',
    category: 'Object & Infrastructure Cloud',
    isSimulated: true,
    defaultZeroEgress: false,
    badgeColor: '#38BDF8',
    defaultEndpoint: 'https://storage.googleapis.com',
    defaultRegion: 'us-central1',
    defaultAllocatedGB: 1000,
    baseLatencyMs: 100,
    minBwMBps: 10,
    maxBwMBps: 25,
    partSizeMiB: 8,
    description: 'Simulated multi-regional GCS archive & bucket',
  },
  azure: {
    label: 'Azure Blob Storage',
    shortLabel: 'Azure Blob',
    category: 'Object & Infrastructure Cloud',
    isSimulated: true,
    defaultZeroEgress: false,
    badgeColor: '#38BDF8',
    defaultEndpoint: 'https://televault.blob.core.windows.net',
    defaultRegion: 'eastus2',
    defaultAllocatedGB: 1000,
    baseLatencyMs: 86,
    minBwMBps: 12,
    maxBwMBps: 28,
    partSizeMiB: 8,
    description: 'Simulated Microsoft Azure Hot/Cool Blob container',
  },
  b2: {
    label: 'Backblaze B2 Cloud',
    shortLabel: 'Backblaze B2',
    category: 'Object & Infrastructure Cloud',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#F43F5E',
    defaultEndpoint: 'https://s3.us-west-004.backblazeb2.com',
    defaultRegion: 'us-west-004',
    defaultAllocatedGB: 1500,
    baseLatencyMs: 75,
    minBwMBps: 13,
    maxBwMBps: 29,
    partSizeMiB: 8,
    description: 'Simulated Backblaze B2 S3-compatible object bucket',
  },
  wasabi: {
    label: 'Wasabi Hot Cloud Storage',
    shortLabel: 'Wasabi',
    category: 'Object & Infrastructure Cloud',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#10B981',
    defaultEndpoint: 'https://s3.us-east-1.wasabisys.com',
    defaultRegion: 'us-east-1',
    defaultAllocatedGB: 2000,
    baseLatencyMs: 66,
    minBwMBps: 15,
    maxBwMBps: 33,
    partSizeMiB: 8,
    description: 'Simulated Wasabi zero-egress hot object storage',
  },
  do_spaces: {
    label: 'DigitalOcean Spaces',
    shortLabel: 'DO Spaces',
    category: 'Object & Infrastructure Cloud',
    isSimulated: true,
    defaultZeroEgress: false,
    badgeColor: '#3B82F6',
    defaultEndpoint: 'https://nyc3.digitaloceanspaces.com',
    defaultRegion: 'nyc3',
    defaultAllocatedGB: 500,
    baseLatencyMs: 70,
    minBwMBps: 13,
    maxBwMBps: 28,
    partSizeMiB: 8,
    description: 'Simulated DigitalOcean Spaces CDN object bucket',
  },
  nextcloud: {
    label: 'Nextcloud Hub',
    shortLabel: 'Nextcloud',
    category: 'Self-Hosted & Protocol',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#0284C7',
    defaultEndpoint: 'https://cloud.internal-node.example/remote.php/dav',
    defaultRegion: 'self-hosted',
    defaultAllocatedGB: 1000,
    baseLatencyMs: 110,
    minBwMBps: 8,
    maxBwMBps: 20,
    partSizeMiB: 4,
    description: 'Simulated self-hosted Nextcloud WebDAV mount',
  },
  hetzner: {
    label: 'Hetzner Storage Box',
    shortLabel: 'Hetzner Box',
    category: 'Self-Hosted & Protocol',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#EF4444',
    defaultEndpoint: 'https://u123456.your-storagebox.de',
    defaultRegion: 'de-falkenstein',
    defaultAllocatedGB: 5000,
    baseLatencyMs: 90,
    minBwMBps: 11,
    maxBwMBps: 26,
    partSizeMiB: 4,
    description: 'Simulated Hetzner RAID-Z Storage Box mount',
  },
  webdav: {
    label: 'WebDAV Mount',
    shortLabel: 'WebDAV',
    category: 'Self-Hosted & Protocol',
    isSimulated: true,
    defaultZeroEgress: true,
    badgeColor: '#A78BFA',
    defaultEndpoint: 'https://dav.internal-studio.example/remote.php/webdav',
    defaultRegion: 'eu-central',
    defaultAllocatedGB: 500,
    baseLatencyMs: 140,
    minBwMBps: 4,
    maxBwMBps: 12,
    partSizeMiB: 4,
    description: 'Simulated HTTP/WebDAV remote directory mount',
  },
};

export const MOUNTABLE_PROVIDER_KINDS: Exclude<ProviderKind, 'local'>[] = [
  'mega',
  'terabox',
  'gdrive',
  'dropbox',
  'onedrive',
  'box',
  'pcloud',
  'icloud',
  'proton',
  'sync',
  'kdrive',
  's3',
  'r2',
  'gcs',
  'azure',
  'b2',
  'wasabi',
  'do_spaces',
  'nextcloud',
  'hetzner',
  'webdav',
];

export const FILE_CATEGORIES: { id: FileCategory; label: string }[] = [
  { id: 'all', label: 'All' },
  { id: 'images', label: 'Images' },
  { id: 'documents', label: 'Documents' },
  { id: 'audio', label: 'Audio' },
  { id: 'video', label: 'Video' },
  { id: 'code', label: 'Code' },
  { id: 'archives', label: 'Archives' },
  { id: 'other', label: 'Other' },
];

export function getFileCategory(mimeType: string, extension: string): Exclude<FileCategory, 'all'> {
  const ext = extension.toLowerCase().replace(/^\./, '');
  const mime = mimeType.toLowerCase();

  if (mime.startsWith('image/') || ['png', 'jpg', 'jpeg', 'gif', 'webp', 'svg', 'bmp', 'ico'].includes(ext)) {
    return 'images';
  }
  if (mime.startsWith('audio/') || ['wav', 'mp3', 'ogg', 'flac', 'm4a', 'aac'].includes(ext)) {
    return 'audio';
  }
  if (mime.startsWith('video/') || ['mp4', 'webm', 'mov', 'mkv', 'avi'].includes(ext)) {
    return 'video';
  }
  if (
    ['ts', 'tsx', 'js', 'jsx', 'py', 'sql', 'json', 'yaml', 'yml', 'toml', 'sh', 'rs', 'go', 'css', 'html', 'xml'].includes(ext) ||
    mime.includes('javascript') ||
    mime.includes('json') ||
    mime.includes('sql')
  ) {
    return 'code';
  }
  if (
    ['md', 'txt', 'pdf', 'csv', 'doc', 'docx', 'xls', 'xlsx', 'ppt', 'pptx', 'rtf'].includes(ext) ||
    mime.includes('pdf') ||
    mime.includes('markdown') ||
    mime.includes('csv') ||
    mime === 'text/plain'
  ) {
    return 'documents';
  }
  if (['zip', 'tar', 'gz', 'tgz', '7z', 'rar', 'bz2', 'parquet'].includes(ext) || mime.includes('zip') || mime.includes('tar')) {
    return 'archives';
  }
  return 'other';
}
