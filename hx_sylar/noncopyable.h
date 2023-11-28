//
// Created by hx on 23-11-28.
//

#ifndef HX_SYLAR_NONCOPYABLE_H
#define HX_SYLAR_NONCOPYABLE_H

namespace hx_sylar{
	class Noncopyable{
	public:
		Noncopyable(const Noncopyable&) = delete;
		Noncopyable operator=(const Noncopyable&) = delete;
		Noncopyable() =default;
		~Noncopyable() = default;
	};
}

#endif //HX_SYLAR_NONCOPYABLE_H
