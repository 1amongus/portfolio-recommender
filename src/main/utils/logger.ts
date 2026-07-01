type LogLevel = 'info' | 'warn' | 'error' | 'debug';

function formatMessage(level: LogLevel, context: string, message: string): string {
  return `[${new Date().toISOString()}] [${level.toUpperCase()}] [${context}] ${message}`;
}

export const logger = {
  info: (context: string, message: string) => console.log(formatMessage('info', context, message)),
  warn: (context: string, message: string) => console.warn(formatMessage('warn', context, message)),
  error: (context: string, message: string, error?: unknown) => {
    console.error(formatMessage('error', context, message));
    if (error) console.error(error);
  },
  debug: (context: string, message: string) => {
    if (process.env.NODE_ENV === 'development') {
      console.debug(formatMessage('debug', context, message));
    }
  },
};