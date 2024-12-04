# Common Cron Job Patterns Reference

## Cron Format
```
* * * * * command_to_execute
│ │ │ │ │
│ │ │ │ └─ Day of the week (0-6, Sunday = 0)
│ │ │ └─── Month (1-12)
│ │ └────── Day of the month (1-31)
│ └──────── Hour (0-23)
└────────── Minute (0-59)
```

## Common Time Intervals

### Every Minute
```bash
* * * * * /path/to/command
```

### Every 5 Minutes
```bash
*/5 * * * * /path/to/command
```

### Every 15 Minutes
```bash
*/15 * * * * /path/to/command
```

### Every 30 Minutes
```bash
*/30 * * * * /path/to/command
```

### Hourly (At the start of every hour)
```bash
0 * * * * /path/to/command
```

### Every 2 Hours
```bash
0 */2 * * * /path/to/command
```

### Every 6 Hours
```bash
0 */6 * * * /path/to/command
```

### Daily Patterns

#### Every Day at Midnight
```bash
0 0 * * * /path/to/command
```

#### Every Day at 3 AM
```bash
0 3 * * * /path/to/command
```

#### Twice Daily (at 8 AM and 8 PM)
```bash
0 8,20 * * * /path/to/command
```

### Weekly Patterns

#### Every Sunday at Midnight
```bash
0 0 * * 0 /path/to/command
```

#### Every Monday at 8 AM
```bash
0 8 * * 1 /path/to/command
```

#### Every Weekday (Monday to Friday)
```bash
0 0 * * 1-5 /path/to/command
```

#### Every Weekend (Saturday and Sunday)
```bash
0 0 * * 6,0 /path/to/command
```

### Monthly Patterns

#### First Day of Every Month
```bash
0 0 1 * * /path/to/command
```

#### Last Day of Every Month
```bash
59 23 28-31 * * [ "$(date +\%d -d tomorrow)" = "01" ] && /path/to/command
```

#### Every Quarter (First day of every quarter)
```bash
0 0 1 */3 * /path/to/command
```

## Special Strings
You can also use these special strings instead of the five-field pattern:

```bash
@yearly   (equivalent to "0 0 1 1 *")
@monthly  (equivalent to "0 0 1 * *")
@weekly   (equivalent to "0 0 * * 0")
@daily    (equivalent to "0 0 * * *")
@hourly   (equivalent to "0 * * * *")
@reboot   (runs at startup)
```

## Common Use Cases

### System Maintenance
```bash
# Clear temp files daily at 2 AM
0 2 * * * rm -rf /tmp/*

# System updates weekly on Sunday at 3 AM
0 3 * * 0 apt-get update && apt-get -y upgrade

# Database backup every 6 hours
0 */6 * * * pg_dump database_name > /backup/db-$(date +\%Y\%m\%d-\%H).sql
```

### Log Management
```bash
# Rotate logs weekly
0 0 * * 0 /usr/sbin/logrotate /etc/logrotate.conf

# Clean old logs monthly
0 0 1 * * find /var/log -type f -name "*.log" -mtime +30 -delete
```

### Monitoring
```bash
# Check disk space every hour
0 * * * * /usr/local/bin/check_disk_space.sh

# Monitor service status every 5 minutes
*/5 * * * * systemctl is-active --quiet myservice || systemctl restart myservice
```

## Best Practices

1. Always use absolute paths in cron jobs
2. Redirect output to avoid email notifications:
   ```bash
   0 * * * * /path/to/command >/dev/null 2>&1
   ```
3. Set a proper PATH if needed:
   ```bash
   PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
   ```
4. Use proper error handling and logging:
   ```bash
   0 * * * * /path/to/command >> /var/log/cron.log 2>&1
   ```
5. Test commands manually before adding to crontab
6. Use proper locking mechanisms for long-running jobs:
   ```bash
   0 * * * * flock -n /tmp/script.lock /path/to/script.sh
   ```

## Troubleshooting Tips

1. Verify cron daemon is running:
   ```bash
   systemctl status cron
   ```

2. Check cron logs:
   ```bash
   grep CRON /var/log/syslog
   ```

3. Validate crontab syntax:
   ```bash
   crontab -l | crontab -
   ```

4. Test script permissions:
   ```bash
   chmod +x /path/to/script.sh
   ```

Remember to always test your cron jobs in a safe environment first, and ensure proper error handling and logging are in place.
journalctl -u cron.service