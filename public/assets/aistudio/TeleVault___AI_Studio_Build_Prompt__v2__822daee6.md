# TeleVault — AI Studio Build Prompt (v2)

*Front-end prototype spec · every cloud connector is simulated · replaces implementation\_plan.md*

You are a senior front-end engineer. Build **TeleVault**, a dark-mode, keyboard-first multi-cloud file manager, as a **client-only single-page app**: no backend, and no network calls except Google Fonts. Follow this spec exactly. Where it is silent, pick the simplest option that keeps behavior correct and leave a one-line code comment explaining the choice.

## 0. Ground rules

- Complete, runnable code only: no TODOs, placeholders or “rest unchanged”. TypeScript strict, no `any`.
- Build in the phase order of §13 and keep the app runnable after every phase. If output space runs short, finish phases 1–5 completely rather than all phases shallowly.
- **Be honest in the product.** Show a permanent badge “Prototype · simulated connectors · data stays in this browser” and a “Simulated” tag on every non-local mount. Never imply real cloud access, encryption, billing or email. Never use “production-grade”, “SLA”, “SOC 2” or “data sovereignty” in copy.
- Keep the name in one constant: `APP_NAME = 'TeleVault'`.
- **Out of scope:** sharing and ACLs, audit log, client-side encryption, real authentication, real cloud calls, real payments, folder upload, i18n.

## 1. Stack and routes

React 18, TypeScript, Tailwind CSS, `react-router-dom` (HashRouter), `zustand`, `idb`, `@tanstack/react-virtual`, `cmdk`, `recharts`, `lucide-react`, `react-markdown`. No other dependencies; if one is unavailable, write a small replacement instead of dropping the feature. Lazy-load routes.

Routes: `#/` landing · `#/auth` · `#/app/overview` · `#/app/files/:folderId?` · `#/app/starred` · `#/app/trash` · `#/app/providers` · `#/app/analytics` · `#/app/pricing` · `#/app/settings`. Everything under `/app` redirects to `#/auth` without a session. Folder navigation updates the URL; an unknown or trashed `folderId` redirects to root with a toast.

## 2. Data model

```ts
type ProviderKind = 'local' | 's3' | 'r2' | 'gcs' | 'webdav';
type MountStatus = 'connected' | 'syncing' | 'disconnected' | 'error';

interface ProviderMount {
  id: string;                  // 'local-vault' is built in: cannot be disconnected or removed
  kind: ProviderKind;
  name: string;                // unique, case-insensitive
  endpoint: string | null; region: string | null; bucket: string | null;
  status: MountStatus;
  allocatedBytes: number;      // simulated capacity chosen by the user
  latencyMs: number | null;    // from the last handshake
  zeroEgress: boolean;         // true for r2
  lastSyncedAt: number | null; createdAt: number;
}                              // usedBytes is DERIVED from files, never stored

interface NodeBase {
  id: string; parentId: string | null; name: string; starred: boolean;
  trashedAt: number | null;    // set only on the node the user trashed; descendants inherit
  trashedFromParentId: string | null;
  createdAt: number; updatedAt: number;
}
interface FolderNode extends NodeBase { type: 'folder' }  // virtual; may hold files from any mount
interface FileNode extends NodeBase {
  type: 'file'; providerId: string; mimeType: string; extension: string;
  sizeBytes: number; checksumSha256: string | null;
  hasBlob: boolean;            // false = placeholder asset: size is claimed, no bytes exist
  hasThumb: boolean;
}
type VaultNode = FolderNode | FileNode;

type UploadStatus = 'queued' | 'uploading' | 'paused' | 'completed' | 'cancelled' | 'error';
interface UploadJob {          // in memory only; speed and ETA are computed, never persisted
  id: string; fileName: string; fileSize: number;
  targetFolderId: string | null; targetProviderId: string;
  status: UploadStatus; partSizeBytes: number; totalParts: number; completedParts: number;
  uploadedBytes: number; speedBytesPerSec: number; etaSeconds: number | null;
  errorMessage: string | null; createdAt: number;
}
interface ActivityEvent { id?: number; ts: number; kind: 'ingress' | 'egress'; bytes: number; providerId: string }
interface Account { email: string; displayName: string; passwordHash: string; salt: string; createdAt: number }
interface SubscriptionState { planId: 'starter' | 'pro' | 'enterprise'; billingCycle: 'monthly' | 'annual'; invoices: InvoiceRecord[] }
interface InvoiceRecord { id: string; issuedAt: number; description: string; amountUsd: number } // demo only
```

Design decisions:

- Folders are virtual app metadata and can hold files from different mounts. A file stays on the mount it was uploaded to, so moving never crosses providers.
- A file’s `id` is its immutable object key. Rename, move, star and trash change metadata only and never touch stored bytes.
- Mount `usedBytes` = sum of `sizeBytes` of that mount’s files, trashed ones included until permanently deleted.
- Units are SI everywhere: 1 GB = 10⁹ bytes, 1 TB = 1,000 GB.

## 3. Persistence (IndexedDB `televault_db`, via `idb`)

Stores: `meta` (key-value: schemaVersion, seeded, settings), `accounts`, `mounts`, `nodes` (index `parentId`), `blobs` (key `[mountId, objectKey]` → `Blob`), `thumbs` (key nodeId → `Blob`), `activity` (autoIncrement, index `ts`).

- Store real `Blob`s, never base64 data URLs, and never inside node records, so listing a folder stays cheap.
- Versioned `upgrade` handler (one `case` per version). On `versionchange`, close the connection and ask the user to reload.
- Write-through: update UI state only after the transaction commits. Commit an upload’s blob, node and activity event in one transaction.
- Catch `QuotaExceededError` and surface it as a QUOTA\_EXCEEDED error. Before each upload check `navigator.storage.estimate()`. Call `navigator.storage.persist?.()` at startup inside try/catch.
- If IndexedDB cannot open (private mode, blocked in a preview iframe), fall back to an in-memory repository and show an amber banner: “Storage unavailable — changes won’t survive a reload.”
- `BroadcastChannel('televault')` makes other open tabs refresh after a mutation.
- On startup: seed once (§12), then permanently delete anything trashed more than 30 days ago.
- Settings → “Reset to sample workspace” clears everything except the account, then reseeds.

## 4. Adapters and orchestrator

The UI never imports an adapter. It calls `StorageOrchestrator`, which combines the metadata repository with a registry mapping `mount.kind` to an adapter.

```ts
interface ConnectionReport { ok: boolean; latencyMs: number; canList: boolean; canRead: boolean; canWrite: boolean; message: string }
type StorageErrorCode = 'NETWORK' | 'AUTH' | 'QUOTA_EXCEEDED' | 'NOT_FOUND' | 'DISCONNECTED' | 'ABORTED';
class StorageError extends Error { constructor(public code: StorageErrorCode, message: string, public retryable: boolean) { super(message); } }

interface StorageProviderAdapter {
  readonly kind: ProviderKind;
  readonly partSizeBytes: number;
  testConnection(m: ProviderMount, signal?: AbortSignal): Promise<ConnectionReport>;
  createMultipartUpload(m: ProviderMount, key: string, contentType: string): Promise<{ uploadId: string }>;
  uploadPart(m: ProviderMount, key: string, uploadId: string, partNumber: number, data: Blob, signal: AbortSignal): Promise<{ etag: string }>;
  completeMultipartUpload(m: ProviderMount, key: string, uploadId: string, parts: { partNumber: number; etag: string }[], source: Blob): Promise<void>;
  abortMultipartUpload(m: ProviderMount, key: string, uploadId: string): Promise<void>;
  getObject(m: ProviderMount, key: string): Promise<Blob>;
  deleteObject(m: ProviderMount, key: string): Promise<void>;
  getUsage(m: ProviderMount): Promise<{ usedBytes: number; objectCount: number }>;
}
```

- This mirrors the S3 multipart API, so pause, resume and a real backend later need no UI change. `source` exists only for the simulation: simulated adapters buffer nothing and commit `source` to the `blobs` store in `completeMultipartUpload` (say so in a comment).
- Part sizes: s3 and r2 8 MiB (the real minimum is 5 MiB except the last part), gcs 8 MiB (a multiple of 256 KiB), webdav 4 MiB, local 1 MiB.
- `IndexedDBStorageAdapter` (kind `local`) is real and fails only on quota. `S3Adapter`, `R2Adapter`, `GCSAdapter` and `WebDAVAdapter` extend a shared `SimulatedCloudAdapter` with per-kind latency (s3 80, r2 60, gcs 100, webdav 140 ms, ±30% jitter) and bandwidth (s3 12–30, r2 15–35, gcs 10–25, webdav 4–12 MB/s).
- Settings → Developer panel: failure rate (0 / 10 / 50 %) and an Offline toggle for simulated adapters, so error states can be tested. Offline throws NETWORK (retryable); a disconnected mount throws DISCONNECTED (not retryable).
- Orchestrator methods: `listChildren`, `createFolder`, `rename`, `move`, `trash`, `restore`, `purge`, `emptyTrash`, `toggleStar`, `startUpload`, `downloadFile`, `getObjectUrl`, `testMount`, `addMount`, `updateMount`, `disconnectMount`, `reconnectMount`, `removeMount`, `resetWorkspace`.

File-operation rules:

- **Names:** trimmed, 1–255 chars, no `/`, unique among siblings (case-insensitive). Rename and New Folder show an inline error on collision; upload, move and restore auto-suffix `  (1) `, `  (2) `.
- **Move:** target must be a non-trashed folder and not the item itself or one of its descendants.
- **Trash:** sets `trashedAt` and `trashedFromParentId` on that node only, with a 6 s Undo toast. The Trash view lists nodes that have `trashedAt`. **Restore** goes to the original parent if it still exists and is not trashed, otherwise to root. **Delete forever** and **Empty trash** ask for confirmation and call `deleteObject` for every file removed.
- **Disconnected mount:** its files stay visible but dimmed with a “Disconnected” badge; preview, download and uploads to it are disabled with a tooltip. Reconnect reruns the handshake. **Remove mount** confirms, states how many files will be deleted, then deletes them.
- **Placeholder files** (`hasBlob: false`) show an info state, no preview and a disabled Download.

## 5. Upload engine

- Entry points: Upload button, OS drag-and-drop (only when `dataTransfer.types` includes `Files`; dropped folders are ignored with a toast), and the Overview drop zone.
- **Configurator modal:** file list; target folder picker (defaults to the current folder); provider select showing connected mounts only, with free space (`allocatedBytes − used`), disabling a mount the file would not fit in. Max 200 MB per file; larger files are rejected in the list with a reason.
- Concurrency 3; extra jobs wait as `queued`.
- Per job: `createMultipartUpload`, then `uploadPart` for each part in order, then `completeMultipartUpload`. Every part, including the only part of a small file, reports progress ticks every 100 ms so the bar moves smoothly. Retry a failed retryable part up to 3 times with 500 ms · 2ⁿ backoff, then set `error` with a Retry button that resumes at the first incomplete part.
- **Pause** aborts the in-flight part; **Resume** restarts at the first incomplete part, so progress may step back slightly (as real multipart does). **Cancel** calls `abortMultipartUpload` and leaves no node behind. Allowed transitions: queued→uploading, uploading⇄paused, uploading→completed | error | cancelled, queued | paused→cancelled, error→queued.
- **Speed** = bytes completed over a 3 s sliding window, shown in `MB/s`. **ETA** = remaining ÷ speed, “—” when speed is 0.
- On completion: create the `FileNode` (collision-safe name); compute SHA-256 with `crypto.subtle.digest` for files ≤ 64 MB, otherwise store `null` and show “Not computed”; generate a 320 px PNG thumbnail for images; record ingress activity.
- After a page reload, any non-completed job becomes a failed job: “Interrupted by reload — upload again”.
- **Transfer tray:** bottom-right, collapsible, on every `/app` route. Header: active count and aggregate speed. Row: name, provider, part x/y, progress bar (animate with `transform: scaleX`), speed, ETA, Pause / Resume / Cancel / Retry. “Clear completed” button. `aria-live=polite`, announcing only completed and failed, never percentages.

## 6. File explorer

- **Views:** Linear-style table (40 px rows; Name, Provider, Size, Modified, Star) and a Drive-style card grid; remember the choice. The table is virtualised. Folders sort first; click a header to sort (`aria-sort`).
- **Filters:** category chips (All, Images, Documents, Audio, Video, Code, Archives, Other) and a provider select.
- **Selection:** click, Shift-click range, Ctrl/⌘-click toggle, Esc clears. A batch toolbar offers Move, Star, Download, Trash. Multi-file download runs one file at a time (no zip).
- **Keyboard:** ↑/↓ or J/K move; Enter opens (folder navigates, file opens inspector); Space toggles preview; Ctrl/⌘+A select all; F2 rename; S star; Delete/Backspace trash; `/` focuses search; `?` shows a shortcuts sheet; Ctrl/⌘+K opens the command palette. Show ⌘ on Apple platforms and Ctrl elsewhere.
- **Context menu** (right-click): Open, Preview, Rename, Star, Move to…, Download, Move to Trash.
- **Sidebar tree** (`role=tree`, arrow keys expand/collapse) and a breadcrumb that collapses its middle segments beyond 4.
- **Inspector** (320 px) and preview modal show name, type, size, provider, path, dates, SHA-256 and actions. Previews: image, audio and video via object URLs; text, Markdown, JSON, CSV and code read as text up to 1 MB (Markdown through `react-markdown` without raw HTML; code in a monospace block with line numbers); PDF via `<object>`; HTML files shown as plain text, never rendered; SVG only through `<img>`; anything else gets an icon and Download. Create object URLs on open and revoke them on close.
- **Grid thumbnails:** stored thumbnail for images, file-type icon otherwise.
- Internal drag-to-move (rows onto folders or tree nodes) is optional; skip it if space is tight.
- Every list has loading (skeleton, 300 ms minimum on navigation), empty and error states.

## 7. Other screens

- **Landing:** sticky nav; hero with a static, non-interactive explorer preview; six-card bento grid; “How it works” adapter-layer diagram including the prototype note; pricing estimator (reuse §8 component); footer. CTAs: “Launch demo workspace” (creates or reuses the demo account and enters the app) and “Sign in”.
- **Auth:** Sign in, Create account, Reset password, Instant demo. Local demo accounts only: password stored as a PBKDF2-SHA256 hash (600,000 iterations, random 16-byte salt) via WebCrypto, with the text “Local demo account — not a real login”. Reset password checks the email exists locally and lets the user set a new password; no email is sent, and the screen says so.
- **Overview:** KPI cards (managed storage used / plan limit, connected mounts, egress this month, estimated monthly egress savings); mount list with usage bars, status badge and latency; 10 newest files; pinned folders (= starred folders); active-transfer summary; drop zone.
- **Providers:** a card per mount (kind, bucket or endpoint, region, status badge with text, used/allocated, latency, Simulated tag) with Test connection, Edit, Disconnect/Reconnect, Remove. **Add mount** form per kind: s3 (region, bucket), r2 (account ID → endpoint `https://<accountId>.r2.cloudflarestorage.com`, bucket), gcs (project ID, bucket, location), webdav (URL, base path), plus name and allocated capacity in GB. **Do not collect secret keys or passwords.** Bucket names: s3 3–63 lowercase letters, digits, dots, hyphens; r2 3–63 lowercase letters, digits, hyphens; gcs 3–63 lowercase letters, digits, dots, hyphens, underscores; all must start and end with a letter or digit. WebDAV URL must be a valid http(s) URL. The handshake shows steps (“Resolving endpoint…”, “Authenticating (simulated)…”, “Checking permissions…”) over about 1.5 s and records latency.
- **Analytics:** capacity by file type and by provider (computed from real nodes); ingress/egress area chart with 7 D / 30 D toggle built from seeded sample history plus real activity events, labelled “Includes sample history”; zero-egress savings card.
- **Settings:** profile, demo API tokens (generate random, copy, revoke; labelled demo), storage status (`navigator.storage.estimate()`, persisted yes/no), Developer panel, Reset workspace, Sign out.
- **Command palette (`cmdk`):** files and folders (fuzzy name match, max 8), mounts, pages, actions (Upload files, New folder, Toggle view). Esc closes, Enter runs, focus returns to the previous element.

## 8. Pricing (illustrative demo numbers in `pricing.ts`)

|  | Starter | Pro Vault | Enterprise Mesh |
| --- | --- | --- | --- |
| Base / month | $0 | $19 | $79 |
| Included managed storage | 50 GB | 2 TB | 20 TB |
| Included egress / month | 100 GB | 1 TB | 10 TB |
| Mounts | 2 | 10 | Unlimited |
| Seats | 1 | 3 | 10 |

- Annual billing = 20 % off the base price.
- Paid plans: `cost = base + max(0, storageGB − includedStorage) × 0.015 + max(0, egressGB − includedEgress) × 0.03`. Starter is hard-capped; the estimator shows “Upgrade required” past its limits.
- Estimator: two sliders (storage 0–50 TB, egress 0–20 TB) show the cost on every plan and highlight the cheapest plan that fits.
- Switching plans updates limits instantly, adds a demo invoice and shows a toast. A downgrade the workspace already exceeds (used storage or mount count) is blocked with a message saying why. The demo account starts on Pro Vault, monthly.
- Egress savings = this month’s egress on zero-egress mounts × the S3 rate. Keep USD/GB rates in `pricing.ts` (s3 0.09, gcs 0.12, r2 0, webdav 0, local 0) with a comment that they are illustrative and must be verified before real use.

## 9. Design system

- Dark only, `color-scheme: dark`. Tokens as CSS variables, mapped in Tailwind.
- **Canvas** `#090A0F` app, `#0E1017` workspace. **Surface** `#131620`. **Hairline** `#1E2230` and `rgba(255,255,255,.07)`. **Text** `#F3F4F6`, muted `#8A94A6`.
- **Accent:** filled buttons `#4F46E5` with white text (about 6:1; plain `#6366F1` is right at the 4.5:1 limit). Links, active states and the focus ring `#818CF8`. Focus ring is 2 px with a 2 px offset on every interactive element.
- **Status, always with a text label:** Connected / Completed `#10B981`, Syncing / High usage `#F59E0B`, Disconnected / Failed `#F43F5E`.
- **Type:** `DM Sans` for UI and headings (400/500/700; headings `-0.02em`, `text-wrap: balance`); `JetBrains Mono` with `tabular-nums` for byte counts, speeds, checksums and dates. Google Fonts with `display=swap` and real fallbacks. UI text 13–15 px.
- **Layout:** 256 px collapsible sidebar; 3-zone top bar (breadcrumb · search and view toggle · Upload button); optional 320 px inspector. One elevation level: 1 px borders, no nested shadow cards. Metadata is inline text with middots; only filters and toggles look like buttons.
- **Motion:** 150 ms, animate only `transform` and `opacity`; disable under `prefers-reduced-motion`.
- **Formatters** in `lib/format.ts`: `formatBytes` (SI, one decimal), `formatSpeed`, `formatEta` (`12s`, `3m 20s`), dates relative within 7 days otherwise `Mon D, YYYY` with the full timestamp in a tooltip.

## 10. Accessibility and responsive

- Skip link; visible focus; roving tabindex in table and grid; `role=grid` with `aria-sort`; `role=tree` for folders; dialogs trap focus, close on Esc and return focus; toasts use `aria-live=polite`; icon-only buttons have labels; status is never color alone.
- ≥ 1280 px full layout. 768–1279 px: 64 px icon-rail sidebar, inspector overlays. < 768 px: sidebar in a slide-over, table shows Name and Size only, grid has 2 columns, transfer tray is a bottom sheet. The landing page is responsive at every size.

## 11. Seed data (deterministic: `mulberry32`, seed 42; no network)

- Mounts: `local-vault` (local), `s3-prod`, `r2-edge`, `gcs-archive`, `webdav-nas`. All connected, with different capacities and latencies.
- Folders (up to 3 levels): Engineering/{Releases, Backups, Docs}, Design/{Brand, Product Shots}, Finance/Invoices, Media/Audio, Archive/2024.
- About 36 files spread across mounts: roughly 24 with **real generated content** (PNGs drawn on a canvas, valid 16-bit PCM mono WAV tones, Markdown, `.ts` / `.py` / `.sql` / `.json`, CSV, logs) and roughly 12 **placeholders** (zip, mp4, psd, parquet, pdf) with claimed sizes totalling about 120–150 GB. Real files get true `sizeBytes`, SHA-256 and thumbnails.
- 30 days of sample ingress/egress activity and the demo account on Pro Vault.

## 12. Build order

1. **Foundation:** tokens, fonts, formatters, domain types, IndexedDB repository, seed, orchestrator skeleton.
2. **Shell:** routing, auth, app layout, sidebar, top bar, palette skeleton.
3. **Explorer:** table, grid, tree, breadcrumb, selection, keyboard, file operations, trash, inspector and previews.
4. **Uploads:** adapters, upload engine, configurator, transfer tray.
5. **Providers and Overview:** mount cards, add / edit / test / disconnect / remove, dashboard.
6. **Analytics, Pricing, Settings.**
7. **Landing page, palette polish, accessibility pass, responsive pass.**

## 13. Acceptance checklist (verify before finishing)

1. Reload keeps folders, uploaded files, renames, stars, trash state, mounts, plan and settings.
2. A 5 MB image upload shows smooth progress, then a working preview and download after reload.
3. Pause, resume, cancel and retry behave as in §5; cancel leaves no file.
4. Two items with the same name never coexist in one folder.
5. A folder cannot be moved into itself or its descendants.
6. Trashing a folder hides its contents; restore brings everything back; delete forever frees mount usage.
7. A disconnected mount dims its files and disables preview, download and uploads to it.
8. Ctrl+K and ⌘K both open the palette; every shortcut in §6 works; focus is never lost or trapped.
9. With failure rate 50 % and with Offline on, uploads show errors and Retry works.
10. No file data in `localStorage`; no secrets collected anywhere.
11. Placeholder files never show a broken preview or an enabled download.
12. Used storage, quota meter, analytics and mount usage all agree.
13. Plan switching updates limits and blocks invalid downgrades.
14. The layout works at 1440, 1024 and 390 px with no horizontal page scroll.
15. The prototype badge and Simulated tags are always visible.
