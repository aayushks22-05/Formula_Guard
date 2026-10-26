param(
    [string]$Target = "all"
)

$srcCommon = "src/lexer.c src/parser.c src/ast.c src/semantic.c src/normalize.c src/pattern_analysis.c src/anomaly_detection.c"
$gccFlags = "-Wall -Wextra -pedantic -std=c99 -Isrc"

function Build-And-Run($name, $cmdBuild, $exeName) {
    Write-Host "--------------------------------------------------" -ForegroundColor Cyan
    Write-Host "Building & Running: $name" -ForegroundColor Yellow
    Write-Host "--------------------------------------------------" -ForegroundColor Cyan
    Invoke-Expression $cmdBuild
    if ($LASTEXITCODE -eq 0) {
        Invoke-Expression ".\$exeName"
    } else {
        Write-Host "Build failed for $name" -ForegroundColor Red
    }
    Write-Host ""
}

switch ($Target.ToLower()) {
    "audit" {
        Build-And-Run "FormulaGuard Audit (tests/SEM5.xlsx)" `
            "gcc $gccFlags src/main.c src/sheet_reader.c $srcCommon -o formulaguard.exe" `
            "formulaguard.exe tests/SEM5.xlsx"
    }
    "book1" {
        Build-And-Run "FormulaGuard Audit (tests/Book1.xlsx)" `
            "gcc $gccFlags src/main.c src/sheet_reader.c $srcCommon -o formulaguard.exe" `
            "formulaguard.exe tests/Book1.xlsx"
    }
    "lexer" {
        Build-And-Run "Lexer Tests" `
            "gcc $gccFlags tests/lexer_tests.c src/lexer.c -o test_lexer.exe" `
            "test_lexer.exe"
    }
    "parser" {
        Build-And-Run "Parser Tests" `
            "gcc $gccFlags tests/parser_tests.c src/lexer.c src/parser.c src/ast.c -o test_parser.exe" `
            "test_parser.exe"
    }
    "ast" {
        Build-And-Run "AST Tests" `
            "gcc $gccFlags tests/ast_tests.c src/lexer.c src/parser.c src/ast.c -o test_ast.exe" `
            "test_ast.exe"
    }
    "semantic" {
        Build-And-Run "Semantic Analysis Tests" `
            "gcc $gccFlags tests/semantic_tests.c src/lexer.c src/parser.c src/ast.c src/semantic.c -o test_semantic.exe" `
            "test_semantic.exe"
    }
    "normalize" {
        Build-And-Run "Normalization Tests" `
            "gcc $gccFlags tests/normalize_tests.c src/lexer.c src/parser.c src/ast.c src/normalize.c -o test_normalize.exe" `
            "test_normalize.exe"
    }
    "pattern" {
        Build-And-Run "Pattern Analysis Tests" `
            "gcc $gccFlags tests/pattern_analysis_tests.c src/pattern_analysis.c src/normalize.c src/semantic.c src/parser.c src/lexer.c src/ast.c -o test_pattern_analysis.exe" `
            "test_pattern_analysis.exe"
    }
    "anomaly" {
        Build-And-Run "Anomaly Detection Tests" `
            "gcc $gccFlags tests/anomaly_detection_tests.c src/anomaly_detection.c src/pattern_analysis.c src/normalize.c src/semantic.c src/parser.c src/lexer.c src/ast.c -o test_anomaly_detection.exe" `
            "test_anomaly_detection.exe"
    }
    "all" {
        Write-Host "==================================================" -ForegroundColor Green
        Write-Host "     RUNNING COMPLETE FORMULAGUARD TEST SUITE     " -ForegroundColor Green
        Write-Host "==================================================" -ForegroundColor Green
        & $PSCommandPath lexer
        & $PSCommandPath parser
        & $PSCommandPath ast
        & $PSCommandPath semantic
        & $PSCommandPath normalize
        & $PSCommandPath pattern
        & $PSCommandPath anomaly
        & $PSCommandPath audit
    }
    default {
        if (Test-Path $Target) {
            Build-And-Run "FormulaGuard Audit ($Target)" `
                "gcc $gccFlags src/main.c src/sheet_reader.c $srcCommon -o formulaguard.exe" `
                "formulaguard.exe `"$Target`""
        } else {
            Write-Host "Usage: .\run.ps1 <path_to_any_file.xlsx>" -ForegroundColor Yellow
            Write-Host "Or targets: [all|audit|book1|lexer|parser|ast|semantic|normalize|pattern|anomaly]" -ForegroundColor Magenta
            Write-Host "Example: .\run.ps1 tests/SEM5.xlsx" -ForegroundColor Cyan
            Write-Host "Example: .\run.ps1 my_sheet.xlsx" -ForegroundColor Cyan
        }
    }
}
