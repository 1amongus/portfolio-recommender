export class TokenBucketRateLimiter {
  private tokens: number;
  private lastRefill: number;

  constructor(
    private readonly maxTokens: number,
    private readonly refillRatePerSecond: number,
  ) {
    this.tokens = maxTokens;
    this.lastRefill = Date.now();
  }

  private refill(): void {
    const now = Date.now();
    const elapsed = (now - this.lastRefill) / 1000;
    this.tokens = Math.min(this.maxTokens, this.tokens + elapsed * this.refillRatePerSecond);
    this.lastRefill = now;
  }

  async acquire(): Promise<void> {
    this.refill();
    if (this.tokens >= 1) {
      this.tokens -= 1;
      return;
    }
    // Wait until a token is available
    const waitMs = ((1 - this.tokens) / this.refillRatePerSecond) * 1000;
    await new Promise((resolve) => setTimeout(resolve, Math.ceil(waitMs)));
    this.refill();
    this.tokens -= 1;
  }

  get availableTokens(): number {
    this.refill();
    return this.tokens;
  }
}

// Pre-configured rate limiters for each provider
export const rateLimiters = {
  'alpha-vantage': new TokenBucketRateLimiter(5, 5 / 60), // 5/min
  fmp: new TokenBucketRateLimiter(10, 10 / 60), // 10/min
  polygon: new TokenBucketRateLimiter(5, 5 / 60), // 5/min
  fred: new TokenBucketRateLimiter(30, 30 / 60), // 30/min (generous)
};