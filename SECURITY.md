# Coordinated Vulnerability Disclosure Policy

FirmaChain takes the security of this Ledger app seriously. Because this app handles signing operations for user funds, security vulnerabilities have high impact and must be handled responsibly.

> **IMPORTANT**: *DO NOT* open public issues on this repository for security vulnerabilities.

## Scope

| Scope                 |
|-----------------------|
| last release (tagged) |
| main branch           |

The latest **release tag** of this repository is supported for security updates, as well as the **main** branch. Security vulnerabilities should be reported if they can be reproduced on either one of those.

## Reporting a Vulnerability

Please report vulnerabilities using one of the following methods:

| Reporting methods                                             |
|---------------------------------------------------------------|
| Email: **developer@firmachain.org**                           |
| [GitHub Private Vulnerability Reporting][gh-private-advisory] |

When reporting, please include:

- A description of the vulnerability and its impact
- Steps to reproduce (if applicable)
- Affected version / commit hash
- Any suggested mitigation

If you are not familiar with CVSS, a Low/High/Critical severity rating is sufficient. A partially filled-in report for a critical vulnerability is still better than no report at all.

## Guidelines

We ask all researchers to:

- Abide by this policy to disclose vulnerabilities, and avoid posting vulnerability information in public places (GitHub, Discord, Telegram, Twitter, etc.)
- Make every effort to avoid privacy violations, degradation of user experience, disruption to production systems, and destruction of data
- Keep information about discovered vulnerabilities confidential between yourself and the FirmaChain team until the issue has been resolved and disclosed
- Avoid posting personally identifiable information, privately or publicly

If you follow these guidelines when reporting an issue to us, we commit to:

- Not pursue or support any legal action related to your research on this vulnerability
- Work with you to understand, resolve, and ultimately disclose the issue in a timely fashion

[gh-private-advisory]: /../../security/advisories/new
