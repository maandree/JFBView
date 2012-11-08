/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *  Copyright (C) 2012 Chuan Ji <jichuan89@gmail.com>                        *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *   http://www.apache.org/licenses/LICENSE-2.0                              *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// This file declares PDFDocument, an implementation of the Document abstraction
// using MuPDF.

#ifndef PDF_DOCUMENT_HPP
#define PDF_DOCUMENT_HPP

#include "cache.hpp"
#include "document.hpp"
extern "C" {
#include <mupdf.h>
}
#include <queue>
#include <map>
#include <vector>

// Document implementation using MuPDF.
class PDFDocument: public Document {
 public:
  // Default size of page cache.
  enum { DEFAULT_PAGE_CACHE_SIZE = 5 };

  virtual ~PDFDocument();
  // Factory method to construct an instance of PDFDocument. path gives the path
  // to a PDF file. page_cache_size specifies the maximum number of pages to
  // store in memory. Returns NULL if the file cannot be opened.
  static PDFDocument *Open(const std::string &path,
                           int page_cache_size = DEFAULT_PAGE_CACHE_SIZE);
  // See Document.
  virtual int GetPageCount();
  // See Document.
  virtual const PageSize GetPageSize(int page, float zoom, int rotation);
  // See Document.
  virtual void Render(PixelWriter *pw, int page, float zoom, int rotation);
  // See Document.
  virtual const OutlineItem *GetOutline();
  // See Document.
  virtual int Lookup(const OutlineItem *item);
 private:
  // Actual outline item implementation.
  class PDFOutlineItem: public OutlineItem {
   public:
    virtual ~PDFOutlineItem();
    // See Document::OutlineItem.
    int GetPageNum() const;
    // Factory method to create outline items from a fz_outline. This constructs
    // the entire outline hierarchy.
    static PDFOutlineItem *Build(PDFDocument *parent, fz_outline *src);
   private:
    // The document we belong to.
    PDFDocument *_parent;
    // The fz_outline we represent.
    fz_outline *_src;
    // Whether we are the root of the outline hierarchy.
    bool _is_root;

    // We disallow constructors; use the factory method Build() instead.
    PDFOutlineItem(PDFDocument *parent, fz_outline *src);
    // No reason to allow copy constructor.
    PDFOutlineItem(const PDFOutlineItem &);
    // Recursive construction.
    static void BuildRecursive(PDFDocument *parent, fz_outline *src,
                               std::vector<OutlineItem *> *output);
  };
  friend class PDFOutlineItem;
  // Cache for pdf_page.
  class PDFPageCache: public Cache<int, pdf_page *> {
   public:
    PDFPageCache(int cache_size, PDFDocument *parent);
    virtual ~PDFPageCache();
   protected:
    virtual pdf_page *Load(const int &page);
    virtual void Discard(const int &page, pdf_page * &page_struct);
   private:
    PDFDocument *_parent;
  };
  friend class PDFPageCache;

  // MuPDF structures.
  fz_context *_fz_context;
  pdf_document *_pdf_document;
  // Page cache.
  PDFPageCache *_page_cache;

  // We disallow the constructor; use the factory method Open() instead.
  PDFDocument(int page_cache_size);
  // We disallow copying because we store lots of heap allocated state.
  PDFDocument(const PDFDocument &other);
  PDFDocument &operator = (const PDFDocument &other);

  // Wrapper around pdf_load_page that implements caching. If _page_cache_size
  // is reached, throw out the oldest page. Will also attempt to load the pages
  // before and after specified page. Returns the loaded page.
  pdf_page *GetPage(int page);
  // Constructs a transformation matrix from the given parameters.
  fz_matrix Transform(float zoom, int rotation);
  // Returns a bounding box for the given page after applying transformations.
  fz_bbox GetBoundingBox(pdf_page *page_struct, const fz_matrix &m);
};

#endif
