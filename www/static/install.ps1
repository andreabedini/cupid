# Install the latest release of Cupid on Windows.
#
#   irm https://andreabedini.github.io/cupid/install.ps1 | iex
#
# Downloads the x86-64 Windows archive from the GitHub releases of
# andreabedini/cupid, checks it against the SHA256SUMS of the same release
# and, when gh is installed and signed in, against its build attestation,
# then unpacks it under $env:LOCALAPPDATA\Programs and adds its bin
# directory to the user PATH.  Nothing needs administrator rights.
# Settings, all optional:
#
#   $env:CUPID_VERSION      the version to install, such as 0.2.0; default the latest
#   $env:CUPID_INSTALL_DIR  where the release directory goes; default $env:LOCALAPPDATA\Programs
#
# The same steps by hand, and building from source, are described at
# https://andreabedini.github.io/cupid/docs/install/

$ErrorActionPreference = 'Stop'

$repo = 'andreabedini/cupid'
$site = 'https://andreabedini.github.io/cupid'
$target = 'x86_64-windows'
$installDir = if ($env:CUPID_INSTALL_DIR) { $env:CUPID_INSTALL_DIR } else { Join-Path $env:LOCALAPPDATA 'Programs' }

if (-not [Environment]::Is64BitOperatingSystem) {
    throw "There is no release archive for 32-bit Windows; build from source instead: $site/docs/install/"
}

# -- which version ------------------------------------------------------------

if ($env:CUPID_VERSION) {
    $version = $env:CUPID_VERSION.TrimStart('v')
} else {
    try {
        $latest = Invoke-RestMethod -Uri "https://api.github.com/repos/$repo/releases/latest" -Headers @{ 'User-Agent' = 'cupid-install' }
    } catch {
        throw "Cannot find the latest release of Cupid ($($_.Exception.Message)). If none has been published yet, build from source instead: $site/docs/install/"
    }
    $version = ([string]$latest.tag_name).TrimStart('v')
}

$archive = "cupid-$version-$target.zip"
$base = "https://github.com/$repo/releases/download/v$version"

# -- downloading and checking -------------------------------------------------

$tmp = Join-Path ([IO.Path]::GetTempPath()) "cupid-install-$([Guid]::NewGuid())"
New-Item -ItemType Directory -Path $tmp | Out-Null
try {
    Write-Host "Downloading $archive"
    Invoke-WebRequest -UseBasicParsing -Uri "$base/$archive" -OutFile (Join-Path $tmp $archive)
    Invoke-WebRequest -UseBasicParsing -Uri "$base/SHA256SUMS" -OutFile (Join-Path $tmp 'SHA256SUMS')

    $pattern = '^([0-9A-Fa-f]{64}) [ *]' + [regex]::Escape($archive) + '$'
    $line = Get-Content (Join-Path $tmp 'SHA256SUMS') | Where-Object { $_ -match $pattern } | Select-Object -First 1
    if (-not $line) { throw "SHA256SUMS has no line for $archive" }
    $expected = $line.Substring(0, 64)
    $actual = (Get-FileHash -Algorithm SHA256 -Path (Join-Path $tmp $archive)).Hash
    if ($actual -ne $expected) { throw "$archive does not match SHA256SUMS" }

    # -- provenance -----------------------------------------------------------
    #
    # CI records a build attestation for every release file.  gh can check
    # it, but only when signed in, so the check is made when it can be and
    # said to be skipped otherwise.

    $attested = $false
    if (Get-Command gh -ErrorAction SilentlyContinue) {
        & gh auth token *> $null
        if ($LASTEXITCODE -eq 0) {
            Write-Host "Checking the build attestation with gh"
            $out = & gh attestation verify (Join-Path $tmp $archive) --repo $repo --signer-workflow "$repo/.github/workflows/ci.yml" 2>&1
            if ($LASTEXITCODE -ne 0) {
                $out | ForEach-Object { Write-Host $_ }
                throw "$archive has no valid build attestation from $repo's CI"
            }
            $attested = $true
        }
    }

    # -- installing -----------------------------------------------------------

    $dir = Join-Path $installDir "cupid-$version-$target"
    New-Item -ItemType Directory -Force -Path $installDir | Out-Null
    if (Test-Path $dir) { Remove-Item -Recurse -Force $dir }
    Expand-Archive -Path (Join-Path $tmp $archive) -DestinationPath $installDir
    $bin = Join-Path $dir 'bin'
    if (-not (Test-Path (Join-Path $bin 'cupid.exe'))) { throw "$archive does not contain bin\cupid.exe" }

    # Put this version's bin first on the user PATH, dropping any earlier one.
    $userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
    $rest = @($userPath -split ';' | Where-Object { $_ -and $_ -notmatch '\\cupid-[^\\]+-x86_64-windows\\bin\\?$' })
    [Environment]::SetEnvironmentVariable('Path', (@($bin) + $rest) -join ';', 'User')
    $env:Path = "$bin;$env:Path"

    Write-Host "Installed Cupid $version in $dir"
    Write-Host "Added $bin to your user PATH. It is already on the PATH of this session; other terminals pick it up when reopened."
    if (-not $attested) {
        Write-Host "The checksum matched; the build attestation was not checked because gh is not installed or not signed in."
    }
} finally {
    Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}
