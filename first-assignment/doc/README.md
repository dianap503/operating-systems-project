---
name: README
description: README file for the T1 audit project.
---

# T1 System Audit Project

This project performs a comprehensive audit of the system, gathering various metrics and logs related to filesystems, processes, and system status.

## Project Structure

The project is organized into the following directories:

- **`reports/`**: Root directory for all generated output files and logs.
  - **`reports/fs/`**: Contains filesystem-related reports (e.g., file listings, shell script locations, disk usage).
  - **`reports/process/`**: Contains process-related audit logs (e.g., memory usage, process trees).
  - **`reports/proc/`**: Stores data extracted from the `/proc` filesystem (e.g., CPU info, memory status, system uptime).
  - **`reports/pipeline/`**: Stores outputs from complex command pipelines (e.g., top large files, process sorting, command counts).
- **`doc/`**: Contains project documentation, including the command reference (`doc/T1_commands.md`).
- **`tools/`**: Contains executable scripts for the project, including the main audit script.## Execution

To run the system audit, execute the following command from the project root:

```bash
bash tools/t1_audit.sh
```

## Generated Reports

All generated reports are saved within the `reports/` directory structure, organized by category into subfolders. A final summary of the audit execution, including the runtime and created paths, is saved in:

`reports/T1_summary.txt`

