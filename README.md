# FormulaGuard

### Compiler-Based Spreadsheet Formula Analysis & ML-Assisted Anomaly Detection

FormulaGuard is a **compiler-based system for analyzing spreadsheet formulas structurally** and detecting potentially anomalous formula patterns.

Instead of treating formulas as plain text, FormulaGuard processes them using compiler concepts such as **lexical analysis, parsing, Abstract Syntax Trees (ASTs), and semantic analysis**.

The structured representation produced by the compiler will later be used for **formula normalization, feature extraction, and machine-learning-based anomaly detection**.

---

## Problem

Spreadsheet formulas often follow repeated structural patterns.

For example:

```text
=B2+C2
=B3+C3
=B4+C4
```

These formulas are textually different but structurally similar.

If one formula changes unexpectedly:

```text
=B2+C2
=B3+C3
=B4+D4   ← potentially anomalous
```

simple text comparison may not identify the underlying structural deviation.

FormulaGuard aims to detect such inconsistencies by analyzing **formula structure rather than only formula text**.

---

## How It Works

```text
Formula
   ↓
Lexer
   ↓
Tokens
   ↓
Parser
   ↓
AST
   ↓
Semantic Analysis
   ↓
Normalization
   ↓
Structural Features
   ↓
ML Anomaly Detection
```

The compiler frontend provides the structural understanding, while the planned ML layer will use that information to identify unusual patterns.

---

## Current Implementation

The compiler frontend is implemented in **C** and currently includes:

* Lexical analysis
* Formula grammar
* Recursive-descent parser
* AST construction
* Semantic analysis
* Semantic error classification
* Semantic diagnostics
* Semantic error propagation
* Regression testing

**Current test status: 39 semantic tests passing.**

---

## Tech Stack

### Current

* **C** — compiler implementation
* **Git / GitHub** — version control

### Planned

* **Python** — data processing and ML
* **Pandas / NumPy** — feature and dataset processing
* **Scikit-learn** — ML experimentation
* **Matplotlib** — evaluation and visualization

The exact ML approach will be selected after the dataset and feature representation are developed.

---

## Current Focus

The next major component is **formula normalization and structural representation**.

The goal is to represent formulas such as:

```text
=B2+C2
=B3+C3
=B4+C4
```

as instances of the same underlying structural pattern despite their different cell addresses.

This representation will form the bridge between the compiler frontend and the future anomaly-detection system.

---

## Future Enhancements

* Formula normalization
* Structural formula comparison
* Feature extraction
* Formula anomaly dataset generation
* ML-based anomaly detection
* Spreadsheet dependency analysis
* Workbook-level analysis
* Explainable anomaly detection
* Formula repair suggestions
* Spreadsheet auditing
* Advanced structural/graph-based ML models

---

## Research Direction

The central research direction is to investigate whether **compiler-derived structural representations of spreadsheet formulas can improve anomaly detection compared with raw textual representations**.

The project combines:

**Compiler Design + Spreadsheet Analysis + Structural Representation + Machine Learning**

---

## Vision

FormulaGuard aims to evolve from a formula parser into an intelligent spreadsheet-analysis system capable of identifying formulas that are not only invalid, but **structurally inconsistent with their surrounding spreadsheet logic**.
