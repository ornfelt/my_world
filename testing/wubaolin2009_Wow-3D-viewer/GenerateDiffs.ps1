param (
    [switch]$Recurse = $false  # Optional flag to enable recursive comparison
)

# Basically the same as:
# {my_notes_path}/scripts/files/GenerateDiffsCpp.ps1

# Get the code root directory from environment variables
$codeRootDir = [System.Environment]::GetEnvironmentVariable("code_root_dir")

# Define source and target directories
$targetDir = Join-Path -Path $codeRootDir -ChildPath "Code2/Wow/tools/my_world/testing/wubaolin2009_Wow-3D-viewer"
$sourceDir = Join-Path -Path $codeRootDir -ChildPath "Code2/Wow/tools/my_world/c++/wubaolin2009_Wow-3D-viewer"
$outputDir = Join-Path -Path (Get-Location) -ChildPath "wow_3d_diffs"

# Create the output directory if it doesn't exist
if (-not (Test-Path -Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir | Out-Null
}

# Function to process files
function Process-Files {
    param (
        [string]$currentSourceDir,
        [string]$currentOutputDir,
        [switch]$Recursive
    )

    # Get all files in the source directory (recursively if enabled)
    $files = Get-ChildItem -Path $currentSourceDir -File -Recurse:$Recursive

    foreach ($file in $files) {
        # Construct the corresponding target file path
        $relativePath = $file.FullName.Substring($sourceDir.Length)
        $targetFilePath = Join-Path -Path $targetDir -ChildPath $relativePath

        # Ensure the output directory structure exists
        $outputFileDir = Join-Path -Path $currentOutputDir -ChildPath $relativePath
        $outputFileDirPath = Split-Path -Path $outputFileDir -Parent
        if (-not (Test-Path -Path $outputFileDirPath)) {
            New-Item -ItemType Directory -Path $outputFileDirPath -Force | Out-Null
        }

        # Check if the corresponding target file exists
        if (Test-Path -Path $targetFilePath) {
            # Run git diff and save output to a .diff file
            #$diffFilePath = [System.IO.Path]::ChangeExtension($outputFileDir, ".diff")
            # Important to distinguish header files...
            $diffFilePath = if ($file.Extension -ieq ".h") {
                [System.IO.Path]::ChangeExtension($outputFileDir, "_h.diff")
            } else {
                [System.IO.Path]::ChangeExtension($outputFileDir, ".diff")
            }
            git diff --no-index -- "$($file.FullName)" "$targetFilePath" | Out-File -FilePath $diffFilePath -Encoding UTF8
        } else {
            # Create a placeholder file for the missing target file
            $missingDiffFilePath = Join-Path -Path $outputFileDirPath -ChildPath "MISSING_$($file.Name).diff"
            "File $targetFilePath is missing." | Out-File -FilePath $missingDiffFilePath -Encoding UTF8
        }
    }
}

Process-Files -currentSourceDir $sourceDir -currentOutputDir $outputDir -Recursive:$Recurse

Write-Host "Diff generation completed."

