/*
 * ExtractorRegion.h
 *
 *  Created on: 20.02.2011
 *      Author: Simon Moll
 */

#ifndef EXTRACTORREGION_HPP_
#define EXTRACTORREGION_HPP_

#include <axtor/CommonTypes.h>

#include <vector>

namespace axtor {
	/*
	 * header dominated dominance region with extractor context
	 */
	class ExtractorRegion {
		llvm::BasicBlock * header;
	public:

		ExtractorContext context;
		llvm::BasicBlock * getHeader() const { return header; }

		ExtractorRegion(llvm::BasicBlock * _header, ExtractorContext & _context);

		void dump() const;
		void dump(std::string prefix) const;
		bool verify(llvm::DominatorTree & domTree) const;
		bool contains(llvm::DominatorTree & domTree, const llvm::BasicBlock * block) const; //FIXME: does not check context-boundaries
	};

	typedef std::vector<ExtractorRegion> RegionVector;

}
#endif /* EXTRACTORREGION_HPP_ */
