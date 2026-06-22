# Security Policy

## Supported Versions

Security fixes target the current `master` branch.

## Reporting a Vulnerability

Report vulnerabilities privately by opening a private GitHub issue or contacting the maintainer directly before disclosing publicly. Include:

- Affected commit or file.
- Deployment method (Kanata, Karabiner-Elements, or QMK).
- Reproduction steps.
- Impact and any known mitigations.

## Security Scope

Kanata and Karabiner-Elements run as system daemons with elevated input privileges. Relevant issues include:

- Key remapping configurations that could enable unintended input injection.
- Privilege escalation through kanata's daemon or Karabiner-DriverKit extension.
- Unsafe shell commands embedded in kanata `cmd` actions.

QMK firmware runs on dedicated hardware and has no network surface; physical access is a prerequisite for any firmware-level attack.

Do not include personal data or private device identifiers in public reports.
