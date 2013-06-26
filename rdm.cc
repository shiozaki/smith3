//
// SMITH3 - generates spin-free multireference electron correlation programs.
// Filename: rdm.cc
// Copyright (C) 2012 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@northwestern.edu>
// Maintainer: Shiozaki group
//
// This file is part of the SMITH3 package.
//
// The SMITH3 package is free software; you can redistribute it and\/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// The SMITH3 package is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with the SMITH3 package; see COPYING.  If not, write to
// the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//


#include "constants.h"
#include "active.h"
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <map>

using namespace std;
using namespace smith;



bool RDM::reduce_done(const list<int>& done) const {
  // check if there is a annihilation operator which has creation operators in his right side
  bool out = true;
  for (auto i = index_.begin(); i != index_.end(); ++i) {
    // if non-dagger and not registered in done
    if (!(*i)->dagger() && find(done.begin(), done.end(), (*i)->num()) == done.end()) {
      for (auto j = i; j != index_.end(); ++j) {
        if ((*j)->dagger()) out = false;
      }
      break;
    }
  }
  return out;
}


void RDM::sort() {

  // of course this is not the fastest code, but I am fine.

  // first we align indices so that
  // 0+ 0 1+ 1 2+ 2...
  // actually this might be better for actually implementation.
  vector<shared_ptr<Spin>> done_spin;
  while (!done()) {

    list<shared_ptr<const Index>> buf;
    auto i = index_.begin();
    // continue to spin which is not processed
    for (; i != index_.end(); ++i) {
      if (find(done_spin.begin(), done_spin.end(), (*i)->spin()) != done_spin.end()) {
        buf.push_back(*i);
      } else {
        break;
      }
    }
    {
      shared_ptr<Spin> cs = (*i)->spin();
      const bool dagger = (*i)->dagger();
      auto j = i;
      int cnt = 0;
      bool found = false;

      if (dagger) {
        // if dagger, move it to right before the nondagger of the same spin
        for (++j; j != index_.end(); ++j) {
          if ((*j)->spin() == cs) {
            buf.push_back(*i);
            buf.push_back(*j);
            assert(!(*j)->dagger());
            found = true;
          } else {
            buf.push_back(*j);
            if (!found) ++cnt;
          }
        }
      } else {
        // if nodagger, move it to right after the dagger of the same spin
        for (++j; j != index_.end(); ++j) {
          if ((*j)->spin() == cs) {
            buf.push_back(*j);
            assert((*j)->dagger());
            buf.push_back(*i);
            ++cnt;
            found = true;
          } else {
            buf.push_back(*j);
            if (!found) ++cnt;
          }
        }
      }
      fac_ *= (cnt%2 == 1) ? -1 : 1;
      done_spin.push_back(cs);
    }
    if (index_.size() != buf.size()) {
      for (auto z = buf.begin(); z != buf.end(); ++z) (*z)->print();
      throw logic_error("RDM::sort()");
    }
    index_ = buf;
  }
}


bool RDM::done() const {
  // if operators are aligned as a0+ a0 a1+ a1...
  bool out = true;
  assert((index_.size()&1) == 0); // for sure..

  int cnt = 0;
  shared_ptr<Spin> prev;
  for (auto i = index_.begin(); i != index_.end(); ++i, ++cnt) {
    // even number, then (*i) should be daggered.
    if ((cnt & 1) == 0) {
      if (!(*i)->dagger()) {
        out = false;
        break;
      }
      prev = (*i)->spin();
    } else {
      if ((*i)->dagger() || (*i)->spin() != prev) {
        out = false;
        break;
      }
    }
  }
  return out;
}


// todo add bra ket comparison
bool RDM::operator==(const RDM& o) const {
  bool out = true;
  // compare all rdms of active objects
  out &= fac_ == o.factor();
  out &= index_.size() == o.index().size();
  out &= delta_.size() == o.delta().size();
  if (index_.size() == o.index().size()) {
    for (auto i = index_.begin(), j = o.index().begin(); i != index_.end(); ++i, ++j)
      out &= (*i)->identical(*j);
  } else {
    out &= false;
  }
  return out;
}


