import { describe, expect, it } from 'vitest';
import { validateBucketName, validateWebdavUrl } from '../pages/ProvidersPage';

describe('Storage Provider Configuration Validators', () => {
  describe('validateBucketName', () => {
    it('enforces length constraints (3 to 63 chars)', () => {
      expect(validateBucketName('s3', 'ab')).toBe('Bucket name must be between 3 and 63 characters.');
      expect(validateBucketName('s3', 'a'.repeat(64))).toBe('Bucket name must be between 3 and 63 characters.');
      expect(validateBucketName('s3', 'abc')).toBeNull();
    });

    it('requires bucket to start and end with lowercase letter or digit', () => {
      expect(validateBucketName('s3', '-invalid-')).toBe(
        'Bucket name must start and end with a lowercase letter or digit.'
      );
      expect(validateBucketName('s3', '.invalid')).toBe(
        'Bucket name must start and end with a lowercase letter or digit.'
      );
      expect(validateBucketName('s3', 'valid-name-123')).toBeNull();
    });

    it('enforces provider-specific character sets', () => {
      // S3 allows lowercase, digits, dots, hyphens
      expect(validateBucketName('s3', 'my.valid-bucket.01')).toBeNull();
      expect(validateBucketName('s3', 'invalid_underscore')).toBe(
        'S3 bucket name may only contain lowercase letters, digits, dots (.), and hyphens (-).'
      );
      expect(validateBucketName('s3', 'UPPERCASE')).toBe(
        'Bucket name must start and end with a lowercase letter or digit.'
      );

      // Cloudflare R2 disallows dots (lowercase, digits, hyphens only)
      expect(validateBucketName('r2', 'my-r2-bucket')).toBeNull();
      expect(validateBucketName('r2', 'my.r2.bucket')).toBe(
        'Cloudflare R2 bucket name may only contain lowercase letters, digits, and hyphens (-).'
      );

      // GCS allows dots, hyphens, and underscores
      expect(validateBucketName('gcs', 'my_gcs-bucket.01')).toBeNull();
      expect(validateBucketName('gcs', 'invalid@bucket')).toBe(
        'GCS bucket name may only contain lowercase letters, digits, dots (.), hyphens (-), and underscores (_).'
      );
    });
  });

  describe('validateWebdavUrl', () => {
    it('accepts valid HTTP and HTTPS URLs', () => {
      expect(validateWebdavUrl('https://dav.example.com/webdav')).toBeNull();
      expect(validateWebdavUrl('http://192.168.1.100:8080/remote.php/dav')).toBeNull();
    });

    it('rejects unsupported protocols', () => {
      expect(validateWebdavUrl('ftp://example.com/files')).toBe(
        'Endpoint URL must start with http:// or https://'
      );
      expect(validateWebdavUrl('ws://example.com')).toBe(
        'Endpoint URL must start with http:// or https://'
      );
    });

    it('rejects malformed URLs', () => {
      expect(validateWebdavUrl('not-a-url')).toBe('Please enter a valid http(s) endpoint URL.');
      expect(validateWebdavUrl('')).toBe('Please enter a valid http(s) endpoint URL.');
    });
  });
});
