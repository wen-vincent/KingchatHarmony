import hilog from '@ohos.hilog';

class Logger
{
  private domain : number;
  private prefix : string;
  private format : string = 'kingchat %{public}s %{public}s';

  constructor(prefix)
  {
    this.prefix = prefix;
    this.domain = 0x0000;
  }

  debug(...args: string[]): void {
    hilog.debug(this.domain, this.prefix, this.format, args);
  }
    info(...args: string[]): void {
    hilog.info(this.domain, this.prefix, this.format, args);
  }

    warn(...args: string[]): void {
    hilog.warn(this.domain, this.prefix, this.format, args);
  }

    error(...args: string[]): void {
    hilog.error(this.domain, this.prefix, this.format, args);
  }

}

export default  Logger;
