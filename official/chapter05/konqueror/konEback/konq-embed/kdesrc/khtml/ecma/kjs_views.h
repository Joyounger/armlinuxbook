/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _KJS_VIEWS_H_
#define _KJS_VIEWS_H_

#include "kjs_dom.h"
#include <dom2_views.h>

namespace KJS {


  class DOMAbstractView : public DOMObject {
  public:
    DOMAbstractView(DOM::AbstractView av) : abstractView(av) {}
    ~DOMAbstractView();
    virtual KJSO tryGet(const UString &p) const;
    // no put - all read-only
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
    virtual DOM::AbstractView toAbstractView() const { return abstractView; }
  protected:
    DOM::AbstractView abstractView;
  };

  class DOMAbstractViewFunc : public DOMFunction {
    friend class DOMNode;
  public:
    DOMAbstractViewFunc(DOM::AbstractView av, int i) : abstractView(av), id(i) { }
    Completion tryExecute(const List &);
    enum { GetComputedStyle };
  private:
    DOM::AbstractView abstractView;
    int id;
  };

  KJSO getDOMAbstractView(DOM::AbstractView av);

  /**
   * Convert an object to an AbstractView. Returns a null Node if not possible.
   */
  DOM::AbstractView toAbstractView(const KJSO&);

} // namespace

#endif
