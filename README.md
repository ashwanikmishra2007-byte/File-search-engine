# File Search Engine

**A Modular File Discovery and Content Indexing Engine Built with C++**

File Search Engine is a systems-oriented project designed to explore efficient file discovery, file content processing, and indexing using C++.

The project is built around two independent but interconnected components: **FileFinder** and **FileReader**.

- **FileFinder** locates files across specified filesystem locations and retrieves their paths.
- **FileReader** opens the discovered files, reads their contents, analyzes textual data, and builds an index of words.

Together, these components form the foundation of a file search engine capable of discovering files and organizing their contents for efficient searching.

The project explores filesystem internals, low-level file operations, data structures, indexing algorithms, and system-level programming.

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [Motivation](#motivation)
3. [System Architecture](#system-architecture)
4. [Core Components](#core-components)
   - [FileFinder](#1-filefinder)
   - [FileReader](#2-filereader)
5. [File Discovery and Indexing Workflow](#file-discovery-and-indexing-workflow)
6. [Windows NTFS Integration](#windows-ntfs-integration)
7. [Technology Stack](#technology-stack)
8. [Project Structure](#project-structure)
9. [Current Development Status](#current-development-status)
10. [Development Roadmap](#development-roadmap)
11. [Building and Running](#building-and-running)
12. [Design Principles](#design-principles)
13. [Future Possibilities](#future-possibilities)

---

## Project Overview

Finding a file on a computer is relatively straightforward when its location is known. However, locating files across large filesystems, processing their contents, and searching through information stored inside them presents a more interesting engineering challenge.

This project approaches that challenge by separating file discovery from file content processing.

Instead of combining every operation into one large class, the system uses independent modules with clearly defined responsibilities.

The overall system is intended to support:

- File discovery using filenames and search criteria.
- Retrieval of all matching file paths.
- Reading and processing discovered files.
- Extraction and indexing of words from file contents.
- Efficient lookup through indexed information.
- Support for large filesystems and large collections of documents.

## Motivation

Conventional file-search implementations often perform filesystem traversal whenever a search is initiated.

This can become inefficient when dealing with large directories and filesystems.

Additionally, searching for a file by its name is fundamentally different from searching for information contained inside that file.

This project explores both problems independently.

**File discovery** deals with filesystem structures, directory relationships, file metadata, and path retrieval.

**Content indexing** deals with file I/O, data processing, text extraction, and efficient data structures.

Separating these concerns makes the system easier to develop, test, optimize, and extend.

---

## System Architecture

The project follows a modular architecture consisting of two primary components.

```text
                 USER QUERY
                      |
                      v
             FILE SEARCH ENGINE
                      |
              +-------+-------+
              |               |
              v               |
          FileFinder          |
              |               |
              v               |
       Filesystem Discovery   |
              |               |
              v               |
        Matching File Paths   |
              |               |
              v               |
          FileReader          |
              |               |
              v               |
          File I/O            |
              |               |
              v               |
       Content Processing     |
              |               |
              v               |
          Word Index          |
              |               |
              v               |
        Searchable Data       |
```

The architecture maintains separation between file discovery and content processing.

FileFinder does not need to understand the contents of a file, and FileReader does not need to understand how a file was discovered.

The interface between them is the collection of file paths returned by FileFinder.

## Core Components

### 1. FileFinder

FileFinder is responsible for discovering files across the filesystem.

Its primary responsibility is to locate every file matching the specified search criteria and return the corresponding paths.

#### Responsibilities

- Search specified filesystem locations.
- Discover files and directories.
- Match filenames against search criteria.
- Retrieve complete paths for matching files.
- Handle multiple search results.
- Provide a common search interface independent of the operating system.

#### Architecture

The FileFinder component uses an object-oriented design.

A common `FileFinder` base class defines the shared search functionality, while platform-specific implementations provide the appropriate filesystem search roots and platform-dependent behavior.

For example:

```text
             FileFinder
             (Base Class)
                  |
          +-------+-------+
          |               |
          v               v
      WinFileFinder    LinuxFileFinder
```

The common search algorithm is separated from platform-specific filesystem access.

This makes it possible to extend the system to additional operating systems without rewriting the entire search engine.

#### Windows NTFS Implementation

For Windows, FileFinder is being developed around NTFS filesystem metadata.

Instead of relying exclusively on recursive directory traversal, the Windows implementation explores the NTFS Master File Table (MFT).

It uses Windows filesystem control operations to enumerate filesystem records and extract metadata.

Important concepts include:

- Master File Table (MFT).
- MFT file reference numbers.
- Parent-directory reference numbers.
- USN records.
- `FSCTL_ENUM_USN_DATA`.
- `DeviceIoControl()`.

The extracted metadata can be used to identify files and reconstruct their complete paths through parent-directory relationships.

This approach is intended to provide a more efficient foundation for file discovery on large NTFS volumes.

### 2. FileReader

FileReader is responsible for accessing and processing the contents of files discovered by FileFinder.

It operates independently of the filesystem discovery mechanism.

FileReader receives file paths from FileFinder, opens the corresponding files, reads their contents, and processes the available data.

#### Responsibilities

- Accept file paths from FileFinder.
- Open discovered files.
- Read file contents.
- Process textual information.
- Extract words from file contents.
- Build an index of words.
- Associate indexed words with their corresponding files.

#### Word Indexing

The central objective of FileReader is to transform file contents into structured information that can be searched efficiently.

Rather than repeatedly opening and scanning every file for every content-search query, FileReader aims to build an index of words extracted from the processed files.

Conceptually:

```text
          Discovered File
                 |
                 v
             FileReader
                 |
                 v
             File I/O
                 |
                 v
          Content Processing
                 |
                 v
            Word Extraction
                 |
                 v
             Word Index
                 |
                 v
       Word-to-File Association
```

The resulting index provides the foundation for content-based searches.

#### Separation of Responsibilities

FileReader is deliberately kept separate from FileFinder.

FileFinder discovers files. FileReader processes them.

This separation allows either component to evolve independently and makes the overall architecture more maintainable.

---

## File Discovery and Indexing Workflow

The complete workflow is designed around the interaction between FileFinder and FileReader.

### Step 1: Search Request

The user provides the search criteria, such as a filename and the filesystem location to search.

### Step 2: File Discovery

FileFinder searches the specified filesystem locations and identifies all matching files.

### Step 3: Path Retrieval

FileFinder retrieves the paths of all matching files.

The system does not stop after discovering the first match.

### Step 4: File Processing

The discovered paths are passed to FileReader.

FileReader independently opens each file and reads its contents.

### Step 5: Content Analysis

FileReader processes the available content and extracts relevant words.

### Step 6: Index Construction

The extracted words are organized into an index that associates searchable terms with their corresponding files.

### Step 7: Search

The indexed information provides the foundation for retrieving files based on their contents rather than only their filenames.

---

## Windows NTFS Integration

An important part of this project is exploring filesystem internals through the Windows NTFS implementation.

NTFS maintains filesystem metadata in the Master File Table.

Each file and directory has an associated MFT record containing information about the filesystem object.

FileFinder explores these records through Windows filesystem APIs.

### Important NTFS Concepts

| Concept | Purpose |
|---|---|
| MFT | Stores metadata records for NTFS filesystem objects |
| MFT Record | Represents an individual filesystem object |
| File Reference Number | Identifies an MFT record |
| Parent Reference Number | Identifies the parent directory |
| USN_RECORD | Structure containing metadata returned by USN-related interfaces |
| USN Journal | Maintains a record of filesystem changes |
| FSCTL_ENUM_USN_DATA | Enumerates MFT records through the NTFS interface |

### Metadata Extraction

The enumeration operation returns binary data containing variable-length records.

FileFinder interprets this data, validates record boundaries, and extracts the required metadata.

The extracted information is represented using a dedicated `Filerecord` structure.

### Path Reconstruction

A file's parent reference number identifies its parent directory.

By following parent references repeatedly, the system can reconstruct the directory hierarchy.

A stack-based approach is used in the planned path reconstruction algorithm to collect directory names and construct the complete path in the correct order.

---

## Technology Stack

| Technology | Purpose |
|---|---|
| C++ | Core application development |
| C++17 | Language standard |
| STL | Data structures and algorithms |
| Windows API | Filesystem and file operations |
| NTFS | Windows filesystem metadata |
| `DeviceIoControl()` | Filesystem control operations |
| MFT | Filesystem record enumeration |
| USN interfaces | Filesystem metadata and change information |

---

## Project Structure

The project is organized into independent modules.

An indicative structure is:

```text
FileSearchEngine/
│
├── src/
│   │
│   ├── FileFinder/
│   │   ├── FileFinder.h
│   │   ├── FileFinder.cpp
│   │   ├── WinFileFinder.h
│   │   └── WinFileFinder.cpp
│   │
│   ├── FileReader/
│   │   ├── FileReader.h
│   │   └── FileReader.cpp
│   │
│   ├── Index/
│   │   ├── Index.h
│   │   └── Index.cpp
│   │
│   ├── Search/
│   │   ├── Search.h
│   │   └── Search.cpp
│   │
│   └── main.cpp
│
├── tests/
│
├── README.md
└── .gitignore
```

The actual directory structure may evolve as the implementation develops.

---

## Current Development Status

The project is under active development.

The primary focus is establishing a reliable architecture for filesystem discovery and file content processing.

### FileFinder

- [x] Object-oriented filesystem search architecture.
- [x] Platform-specific Windows implementation.
- [x] NTFS volume access.
- [x] MFT enumeration using `FSCTL_ENUM_USN_DATA`.
- [x] Enumeration continuation handling.
- [x] Binary metadata buffer processing.
- [x] Record validation and metadata extraction.
- [x] Extraction of filename and file-reference information.
- [ ] Complete parent-reference-based path reconstruction.
- [ ] Integration with the complete search index.
- [ ] Persistent filesystem indexing.

### FileReader

The FileReader component is designed to provide the file content processing and word-indexing functionality.

- [ ] File opening and reading integration.
- [ ] Content processing.
- [ ] Word extraction.
- [ ] Word-to-file indexing.
- [ ] Integration with FileFinder.
- [ ] Efficient content lookup.

### Complete Search Engine

- [ ] Integrate file discovery and content processing.
- [ ] Support filename-based searches.
- [ ] Support content-based searches.
- [ ] Improve indexing and lookup performance.
- [ ] Develop a complete user-facing search workflow.

---

## Development Roadmap

### Phase 1: Filesystem Discovery

- Complete NTFS metadata enumeration.
- Implement reliable metadata extraction.
- Reconstruct complete file paths.
- Develop the filename search index.

### Phase 2: File Content Processing

- Implement file reading.
- Process file contents.
- Extract words.
- Associate extracted words with their source files.

### Phase 3: Search Integration

- Connect FileFinder and FileReader.
- Support filename-based searches.
- Support content-based searches.
- Handle multiple matching files.

### Phase 4: Index Optimization

- Optimize data structures.
- Reduce unnecessary file operations.
- Explore persistent indexing.
- Investigate incremental index updates.

### Phase 5: Reliability and Expansion

- Improve error handling.
- Handle large files and large filesystems.
- Improve permission handling.
- Explore additional filesystem implementations.

---

## Building and Running

### Requirements

- Windows operating system for the current NTFS implementation.
- C++17-compatible compiler.
- MinGW-w64 or another compatible C++ toolchain.
- Windows SDK headers and libraries where required.

### Compilation

The build command depends on the current source organization.

For a simple project with source files in a single directory:

```bash
g++ -std=c++17 -O2 -Wall -Wextra src/*.cpp -o FileSearchEngine.exe
```

For a modular project, compile the relevant source files together or use a build system such as CMake.

### Execution

```powershell
.\FileSearchEngine.exe
```

Some NTFS volume operations may require administrator privileges.

---

## Design Principles

The project follows several important software engineering principles.

**1. Separation of Concerns**

File discovery and file content processing are independent responsibilities.

**2. Modularity**

Each component has a well-defined purpose and interface.

**3. Platform Abstraction**

Common search functionality is separated from operating-system-specific filesystem operations.

**4. Efficient Data Structures**

Indexing structures are used to reduce repeated filesystem and content-processing operations.

**5. Explicit Resource Management**

File handles, buffers, and other system resources must have clearly defined ownership and lifetimes.

**6. Incremental Development**

The project is developed by establishing and validating individual components before integrating them into the complete system.

---

## Future Possibilities

The architecture provides opportunities for further development:

- Persistent search indexes.
- Incremental filesystem indexing.
- Advanced filename search.
- Full-text search across indexed documents.
- Search filters based on file metadata.
- Improved handling of large filesystems.
- Additional operating-system and filesystem backends.
- Performance benchmarking against conventional search approaches.

These are development possibilities, not claims about currently implemented functionality.

---

## Project Goals

The primary goal is to develop a modular file search engine while gaining practical experience in:

- Systems programming.
- Filesystem architecture.
- Operating-system interfaces.
- File I/O and binary data processing.
- Data structures and indexing algorithms.
- C++ object-oriented design.
- Resource and memory management.
- Performance engineering.

**File Search Engine — Discover files. Process contents. Build searchable knowledge.**
