//
// SMITH3 - generates spin-free multireference electron correlation programs.
// Filename: rdm.h
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

#ifndef __SRC_RDM_H
#define __SRC_RDM_H

#include <list>
#include <memory>
#include <string>
#include <cassert>
#include <map>
#include "index.h"

namespace smith {

/// Abstract base class for reduced density matrices (RDMs).
class RDM {
  protected:
    /// Prefactor for RDM.
    double fac_;
    /// Operators that constitute RDM.
    std::list<std::shared_ptr<const Index>> index_;
    /// Kronecker's delta, map with two index pointers.
    std::map<std::shared_ptr<const Index>, std::shared_ptr<const Index>> delta_;



    /// Generate entire task code for Gamma RDM summation.
    virtual std::string generate_not_merged(std::string indent, const std::string tlab, const std::list<std::shared_ptr<const Index>>& loop, std::vector<std::string> in_tensors) = 0;
    /// Generates entire task code for Gamma RDM summation with merged object (additional tensor, here fock tensor) multiplication.
    virtual std::string generate_merged(std::string indent, const std::string itag, const std::list<std::shared_ptr<const Index>>& index, const std::list<std::shared_ptr<const Index>>& merged, const std::string mlab, std::vector<std::string> in_tensors, const bool use_blas) = 0;

    /// Makes if statement in delta cases ie index equivalency check line.
    virtual std::string make_delta_if(std::string& indent, std::vector<std::string>& close) = 0;

    /// Replaces tensor labels to more general labels in(x), where x is a counter for in tensors. RDM tensors numbered before merged (fock) tensor. Eg, rdm1 is mapped to in(0), rdm2 -> in(1), and in merged case with max rdm2, f1 -> in(2).
    virtual void map_in_tensors(std::vector<std::string> in_tensors, std::map<std::string,std::string>& inlab) = 0;

    /// Generate get block - source data to be added to target (move block).
    virtual std::string make_get_block(std::string indent, std::string tag, std::string lbl) = 0;
    /// Generate sort_indices which makes array. This version has no addition (or factor multiplication-0111).
    virtual std::string make_sort_indices(std::string indent, std::string tag, const std::list<std::shared_ptr<const Index>>& loop) = 0;

    /// If delta case, also makes index loops then checks to see if merged-or-delta indices are in loops..
    virtual std::string make_merged_loops(std::string& indent, const std::string tag, std::vector<std::string>& close) = 0;
    /// Loops over delta indices in Gamma summation.
    virtual std::string make_sort_loops(const std::string itag, std::string& indent, const std::list<std::shared_ptr<const Index>>& index, std::vector<std::string>& close) = 0;

    // for task summation line
    /// Generates odata (Gamma) part of for summation ie LHS in equations gamma += rdm or gamma += rdm * f1
    virtual std::string make_odata(const std::string itag, std::string& indent, const std::list<std::shared_ptr<const Index>>& index) = 0;
    /// Generates RDM and merged (fock) tensor multipication.
    virtual std::string multiply_merge(const std::string itag, std::string& indent,  const std::list<std::shared_ptr<const Index>>& merged) = 0;
    /// Adds merged (fock) tensor with indices, used by muliply_merge member.
    virtual std::string fdata_mult(const std::string itag, const std::list<std::shared_ptr<const Index>>& merged) = 0;

    /// Do blas multiplication of Gamma and fock tensors...not implemented yet for subtask code!
    virtual std::string make_blas_multiply(std::string indent, const std::list<std::shared_ptr<const Index>>& loop, const std::list<std::shared_ptr<const Index>>& index) = 0;
    /// Used for blas multiplication of RDM and merged (fock) tensors. NB not implemented yet for subtask code!
    virtual std::pair<std::string, std::string> get_dim(const std::list<std::shared_ptr<const Index>>& di, const std::list<std::shared_ptr<const Index>>& index) const = 0;
 

  public:
    /// Make RDM object from list of indices, delta indices and factor.
    RDM(const std::list<std::shared_ptr<const Index>>& in,
        const std::map<std::shared_ptr<const Index>, std::shared_ptr<const Index>>& in2,
        const double& f = 1.0)
      : fac_(f), index_(in), delta_(in2) { }
    virtual ~RDM() { }


    /// Sort indices so that it will be 0+0 1+1 ... (spin ordering is arbitrary).
    void sort();

    /// Returns the factor
    double factor() const { return fac_; }
    /// Returns a reference to the factor
    double& fac() { return fac_; }

    /// Returns a reference of index_.
    std::list<std::shared_ptr<const Index>>& index() { return index_; }
    /// Returns a const reference of index_.
    const std::list<std::shared_ptr<const Index>>& index() const { return index_; }

    /// Returns a const reference of delta_.
    const std::map<std::shared_ptr<const Index>, std::shared_ptr<const Index>>& delta() const { return delta_; }
    /// Returns a reference of delta_.
    std::map<std::shared_ptr<const Index>, std::shared_ptr<const Index>>& delta() { return delta_; }

    /// Returns if this is in the final form..ie aligned as a0+ a0 a1+ a1..Member function located in active.cc
    bool done() const;
    /// Checks if there is an annihilation operator with creation operators to the right hand side.
    bool reduce_done(const std::list<int>& done) const;

    /// Returns an integer representing rdm rank value, ie (index size)/2
    int rank() const { assert(index_.size()%2 == 0); return index_.size()/2; }

    /// Compares for equivalency based on prefactor, indices and delta.
    bool operator==(const RDM& o) const;


    // virtual public functions
    /// Prints RDM with indentation and prefactors, located in active.cc
    virtual void print(const std::string& indent = "") const = 0;

    /// Application of Wick's theorem and is controlled by const Index::num_. See active.cc. One index is going to be annihilated. done is updated inside the function.
    virtual std::list<std::shared_ptr<RDM>> reduce_one(std::list<int>& done) const = 0;

    /// Generate Gamma summation task, for both non-merged and merged case (RDM * f1 tensor multiplication).
    virtual std::string generate(std::string indent, const std::string itag, const std::list<std::shared_ptr<const Index>>& index, const std::list<std::shared_ptr<const Index>>& merged, const std::string mlab, std::vector<std::string> in_tensors, const bool use_blas) = 0;

    /// Copies this rdm, function located in active.cc
    virtual std::shared_ptr<RDM> copy() const = 0;

};

}

#endif
