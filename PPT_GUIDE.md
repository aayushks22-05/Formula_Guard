# FormulaGuard: Presentation & Slide Deck Preparation Guide

This document is structured specifically to help you build a professional, high-scoring PowerPoint / Slide Deck for your project presentation and technical viva. Each section corresponds to a slide, complete with **Slide Title**, **Visual Recommendations**, **Bullet Points**, and **Speaker Notes** (what to actually say during the presentation).

---

## Slide 1: Title Slide
- **Title**: FormulaGuard: Static Analysis & Structural Anomaly Detection Engine for Spreadsheets
- **Subtitle**: Automated Linting, AST Normalization & Relative Vector Equivalence in C99
- **Presenter Details**: [Your Name / Roll No / Department / Date]
- **Visual**: A clean logo or screenshot of the terminal audit report highlighting `STATUS: 1 POTENTIAL ANOMALY DETECTED`.
- **Speaker Notes**:
  > *"Good morning everyone. Today I am presenting FormulaGuard, a high-performance static analysis engine written in C99 designed to automatically audit spreadsheet formulas, detect structural anomalies, and suggest corrections before silent calculation errors cause business or financial damage."*

---

## Slide 2: The Problem: Spreadsheet Formula Integrity
- **Slide Title**: Why Formula Auditing Matters
- **Key Points**:
  - **The Silent Error Epidemic**: Over 88% of business spreadsheets contain human-introduced errors (Panko's Spreadsheet Research).
  - **Why Excel Fails Users**: Excel only catches fatal syntax errors (`#VALUE!`, `#REF!`, `#DIV/0!`). It remains completely blind to semantic or structural typos (e.g., typing `=COUNT(A10:E10)` instead of `=SUM(A10:E10)`).
  - **Financial & Regulatory Risks**: Famous real-world spreadsheet errors have caused multi-million dollar losses in banking, accounting, and academic research.
- **Visual**: Graphic or split comparison showing a formula producing a valid number (`55`) versus an incorrect formula that still looks valid (`10`), causing an undetected calculation error.
- **Speaker Notes**:
  > *"When an accountant or engineer drags formulas down a column of 100 rows, an accidental modification—such as typing COUNT instead of SUM—evaluates cleanly to a number without throwing an Excel error. Excel says everything is fine, yet the totals are entirely wrong. FormulaGuard was built to solve this exact problem."*

---

## Slide 3: The Solution: FormulaGuard
- **Slide Title**: Introducing FormulaGuard
- **Key Points**:
  - **Static Analysis Engine**: Audits spreadsheets before calculations are trusted.
  - **Zero External Runtime Dependencies**: Built from scratch in pure ANSI/ISO C99 for blazing performance and portability.
  - **Dual-Format Ingestion**: Audits native OpenXML Microsoft Excel (`.xlsx`) and `.csv` files.
  - **Relative Vector Normalization**: Compares formulas based on structural intent rather than literal cell strings.
  - **Smart Dominance Engine**: Distinguishes intentional design patterns from genuine anomalies.
- **Visual**: Diagram showing Spreadsheet $\to$ FormulaGuard Engine $\to$ Diagnostic Report & Repair Suggestions.
- **Speaker Notes**:
  > *"Instead of relying on heavy Python libraries or manual inspections, FormulaGuard operates like a compiler frontend: it lexes, parses, validates, and normalizes formulas into canonical vector structures, giving instant visibility into outliers."*

---

## Slide 4: End-to-End System Architecture
- **Slide Title**: System Pipeline & Workflow
- **Key Points**:
  - **Stage 1 (Ingestion)**: OpenXML decompression and shared formula resolution (`convert_xlsx.py` + `sheet_reader.c`).
  - **Stage 2 (Lexing)**: Tokenization into identifiers, operators, and references (`lexer.c`).
  - **Stage 3 (Parsing)**: Precedence climbing into Abstract Syntax Trees (`parser.c` & `ast.c`).
  - **Stage 4 (Semantic Typing)**: Strict validation of functions and argument types (`semantic.c`).
  - **Stage 5 (Normalization)**: Coordinate vector displacement conversion (`normalize.c`).
  - **Stage 6 (Statistical Anomaly Detection)**: Dual-axis baseline clustering (`anomaly_detection.c` & `main.c`).
- **Visual**: Flowchart diagram illustrating the 6-stage pipeline (reproduced from `ARCHITECTURE.md`).
- **Speaker Notes**:
  > *"Here is our end-to-end architecture. The pipeline behaves like a modern compiler pipeline: from raw OpenXML XML extraction to AST generation, semantic typing, vectorization, and finally statistical regional analysis."*

---

## Slide 5: Technical Deep-Dive: Abstract Syntax Tree (AST)
- **Slide Title**: Parsing Formulas into Abstract Syntax Trees
- **Key Points**:
  - **Precedence Climbing Parser**: Correctly respects mathematical operator hierarchy:
    $$\text{Parentheses/Functions} > \text{Range } (:) > \text{Multiplication/Division} > \text{Addition/Subtraction} > \text{Comparisons}$$
  - **Recursive AST Construction**:
    - Leaf Nodes: Literals, Cell Coordinates, Function Names.
    - Operator Nodes: Left/Right child pointers for binary operators (`+`, `-`, `*`).
    - N-ary Function Nodes: Dynamic child arrays for multi-argument functions (`SUM(A1:B10, 50, C1)`).
- **Visual**: Tree diagram showing `=SUM(COUNT(B7:C7)*50, COUNT(D7:F7)*10)` broken down into root `SUM` node with two `*` children.
- **Speaker Notes**:
  > *"We don't do naive regex search. We parse every formula into a rigorous Abstract Syntax Tree. This allows FormulaGuard to understand nested logic like multiplying a COUNT by a constant inside a SUM."*

---

## Slide 6: The Secret Sauce: Relative Vector Normalization
- **Slide Title**: Structural Equivalence via Vector Normalization
- **Key Points**:
  - **The Coordinate Dilemma**: `=SUM(A1:E1)` at `F1` and `=SUM(A2:E2)` at `F2` look like completely different strings to a computer.
  - **The Mathematical Solution**: Convert every target coordinate into a displacement vector relative to the host cell:
    $$\Delta\text{Col} = \text{TargetCol} - \text{BaseCol}, \quad \Delta\text{Row} = \text{TargetRow} - \text{BaseRow}$$
  - **Resulting Canonical Signature**:
    - Cell `F1`: `SUM(RANGE(REL[-5,0], REL[-1,0]))`
    - Cell `F2`: `SUM(RANGE(REL[-5,0], REL[-1,0]))`
  - **Instant Equivalence**: Identical structural signatures immediately cluster into the same pattern group!
- **Visual**: Side-by-side comparison of raw formulas vs normalized vectors.
- **Speaker Notes**:
  > *"This is the core algorithmic breakthrough of FormulaGuard. By computing relative column and row deltas, all 50 formulas dragged down a column collapse into a single canonical vector string. Any formula that deviates from this vector is mathematically exposed."*

---

## Slide 7: Supported Function Library & Semantic Typing
- **Slide Title**: Robust Semantic Type System
- **Key Points**:
  - Supported Functions:
    - **Aggregations**: `SUM`, `COUNT`, `COUNTA`, `PRODUCT`, `MAX`, `MIN`, `AVERAGE`
    - **Conditionals**: `IF` (Strict boolean condition typing)
    - **Mathematical Transformations**: `ABS`, `ROUND`
  - **Type Checking Enforcement**:
    - Ensures operators only receive compatible types (`SEM_NUMBER`, `SEM_CELL`, `SEM_RANGE`, `SEM_BOOLEAN`).
    - Catches unknown function typos and malformed argument signatures before analysis.
- **Visual**: A clean 2x2 grid displaying Function Categories, Accepted Types, and Return Types.
- **Speaker Notes**:
  > *"Our semantic engine acts as a type linter. It validates that functions receive the correct number and types of arguments, supporting 10 standard Excel functions and all comparison and arithmetic operators."*

---

## Slide 8: Smart Dominance & Multi-Cell Outlier Detection
- **Slide Title**: Anomaly Detection Logic: Dominance vs Outliers
- **Key Points**:
  - **Regional Dominance Baseline ($\ge 60\%$)**:
    - A pattern must command at least 60% of a column/row to become the official baseline standard.
  - **Single-Cell Outliers**: Any isolated formula pattern ($\text{Count} \le 1$) is immediately flagged.
  - **Multi-Cell Cluster Anomaly Detection**:
    - If a dominant pattern has $\ge 75\%$ regional share, a minority cluster of $\le 25\%$ (e.g. 8 `SUM` vs 2 `COUNT` in a 10-cell series) is flagged as an outlier (`[OUTLIER]`).
  - **Contested Regions**: Closer splits (e.g. 3 vs 2) are preserved as `CONTESTED / NO DOMINANT PATTERN` to prevent false alarms on intentional alternating rows.
- **Visual**: Chart showing an 80% vs 20% distribution cleanly separating into `[DOMINANT]` and `[OUTLIER]`.
- **Speaker Notes**:
  > *"A critical challenge in formula linting is handling multi-cell errors—like copying an incorrect formula twice. FormulaGuard solves this using smart dominance thresholding: an 80% dominant baseline treats a 20% minority cluster as an anomaly, while avoiding false alarms on true 50-50 splits."*

---

## Slide 9: Dual-Axis Scanning (Columns & Rows)
- **Slide Title**: Comprehensive 2D Spreadsheet Auditing
- **Key Points**:
  - **Vertical (Column-Wise) Analysis**:
    - Audits formula columns (e.g. row totals, commissions, tax calculations).
  - **Horizontal (Row-Wise) Analysis**:
    - Audits summary footer rows (e.g. quarterly totals, monthly averages).
  - **Smart Filtering**:
    - Automatically ignores isolated single-formula cross-sections to eliminate clutter and focus on actual sequences.
- **Visual**: Visual spreadsheet grid showing a vertical blue scan down Column F and a horizontal orange scan across Row 11.
- **Speaker Notes**:
  > *"Spreadsheets are two-dimensional structures. FormulaGuard audits both axes: vertically down columns where calculations repeat across entities, and horizontally across summary rows at the bottom of data tables."*

---

## Slide 10: Live Demonstration & Case Studies
- **Slide Title**: Real-World Case Studies
- **Case 1: `Book1.xlsx` (Clustered & Row Anomalies)**:
  - **Column F**: 8 `SUM` vs 2 `COUNT` $\to$ Cell `F9` & `F10` flagged as Outliers ($80.0\%$ confidence).
  - **Row 11**: 3 `SUM` vs 1 `MAX` (`D11`) & 1 `COUNT` (`E11`) $\to$ Both flagged for review.
- **Case 2: `SEM5.xlsx` (Nested Formula Constant Anomaly)**:
  - **Column H**: 5 cells contain `SUM(COUNT(...)*15, COUNT(...)*10)`.
  - Cell `H7` contains `*50` instead of `*15` $\to$ Instantly isolated and flagged ($83.3\%$ confidence).
- **Visual**: Actual terminal screenshot showing the colored ANSI report output and actionable repair suggestion.
- **Speaker Notes**:
  > *"Here are real examples from our test suite. In Book1, it cleanly catches both F9 and F10 as outliers in Column F, and catches MAX and COUNT in Row 11. In SEM5, it even catches a single changed multiplication constant (50 instead of 15) deep within a nested formula!"*

---

## Slide 11: Testing, Verification & Code Quality
- **Slide Title**: Verification & Rigorous Testing
- **Key Points**:
  - **Unit Test Suite**: 21 automated assertions testing AST parsing, normalizer output, and edge-case regional distributions.
  - **Automated Regression Suite**: 100% test pass rate with zero compilation warnings under `-Wall -Wextra -pedantic -std=c99`.
  - **Zero Leak Memory Safety**: Complete memory lifecycle management (`init`, `free`, `freeAST`) tested against all regions.
- **Visual**: Terminal snippet showing `================ Results: 21 passed, 0 failed ================`.
- **Speaker Notes**:
  > *"Reliability is paramount. FormulaGuard is backed by comprehensive unit tests covering parsing edge cases, invalid formulas, and memory deallocation routines, compiling cleanly with strict compiler warnings enabled."*

---

## Slide 12: Conclusion & Future Roadmap
- **Slide Title**: Conclusion & Future Extensions
- **Key Points**:
  - **Key Achievements**:
    - Fast, lightweight C99 static formula linter.
    - Robust AST normalization and mathematical vector equivalence.
    - Actionable anomaly reporting with confidence scores and repair suggestions.
  - **Future Roadmap**:
    - Visual Excel Add-in (highlighting cells directly in red/yellow inside Excel).
    - Multi-sheet 3D reference auditing (`Sheet1!A1`).
    - Support for modern array formulas (`FILTER`, `XLOOKUP`).
- **Speaker Notes**:
  > *"In conclusion, FormulaGuard bridges the gap between spreadsheet flexibility and software engineering rigor. Thank you for your time, and I am now ready to take your questions."*

---

## Bonus: Potential Viva / Review Questions & Model Answers

1. **Q: Why write this in C instead of Python with `openpyxl`?**
   - **A**: *"C provides predictable microsecond performance, strict memory control, and can be embedded directly into web assembly, Excel C-APIs, or high-throughput batch servers without Python runtime overhead."*

2. **Q: How do you handle absolute coordinates like `$A$1` vs `A1`?**
   - **A**: *"Our coordinate parser checks for `$` prefixes on both column and row components. In `normalize.c`, relative components compute relative deltas ($\Delta\text{Col}, \Delta\text{Row}$), while absolute components retain an explicit `ABS_CELL($A$1)` signature, correctly identifying when someone forgot to lock a reference."*

3. **Q: What happens if a spreadsheet row intentionally alternates formulas?**
   - **A**: *"FormulaGuard checks the dominance threshold ($60\%$). If a row has 3 SUMs and 2 COUNTs (60% vs 40%), or a 50-50 tie, it declares `REGION_NO_DOMINANT_PATTERN` (contested) rather than flooding the user with false positive anomaly alarms."*
