#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "core/Common.h"
#include "core/Token.h"

#include <unordered_map>
#include <string>

/*
 * Environment Scoping — Linked Chain of Scopes
 *
 * Environments are organized as a linked chain, where each environment
 * holds a pointer to its enclosing (parent) environment.
 *
 * Structure:
 *   Global Env  <-  Block1 Env  <-  Block2 Env
 *                                   (current)
 *
 * Each environment points BACK to its parent, not forward.
 * Global scope has enclosing = nullptr (end of chain).
 *
 * Variable Lookup — walks up the chain:
 *   look up "a" in Block2 → not found
 *   look up "a" in Block1 → not found
 *   look up "a" in Global → found! return it
 *
 * Block Entry — push a new environment:
 *   current = new Environment(current)  // new env, parent = old current
 *
 * Block Exit — pop back to parent:
 *   current = current->enclosing       // restore parent, discard block env
 *
 * Think of it as a STACK — environments are pushed on block entry
 * and popped on block exit. The enclosing pointer is the mechanism
 * that connects them, making it feel like a linked list, but
 * behaving like a stack.
 *
 * This is how all real languages implement lexical scoping —
 * Python, JavaScript, C++ itself all use this same concept.
 */

class Environment {
public:
	/*
	 * Why enclosing is a raw pointer and not a smart pointer:
	 *
	 * enclosing is a NON-OWNING observer — this environment does not own
	 * its parent, it just needs to see it. The parent exists independently
	 * and will always outlive the child.
	 *
	 * Ownership chain:
	 *   Interpreter owns Global Environment (via unique_ptr or direct member)
	 *   Block execution owns its local Environment
	 *   enclosing just OBSERVES the parent — it does not own it
	 *
	 * Why not unique_ptr?
	 *	 nullptr  ←  Global  ←  Block1  ←  Block2
                                   (current)
	 *   unique_ptr implies ownership. If Block1 held a unique_ptr to Global,
	 *   it would DELETE Global when the Block1 ends, as we delete when Block1 ends. 
	 *   That would kill the global scope mid-program. Crash.
	 *
	 * Why not shared_ptr?
	 *   shared_ptr allows multiple owners. It would work, but it's overkill.
	 *   We know exactly who owns what — there is no shared ownership here.
	 *   Using shared_ptr would be misleading about intent.
	 *
	 * raw pointer is the CORRECT and INTENTIONAL choice here:
	 *   unique_ptr  — "I own this, I will delete it"
	 *   shared_ptr  — "we share ownership, last one deletes it"
	 *   raw ptr *   — "I can see this, but I do not own it, I will not delete it"
	 *
	 * This is one of the few legitimate uses of raw pointers in modern C++
	 * — non-owning observers where lifetime is guaranteed by the owner.
	 */

	
	Environment();
	// General rule — always use explicit on single-argument constructors unless you specifically want implicit conversions. Which is almost never.
	explicit Environment(std::shared_ptr<Environment> enclosing);
	
	void define(const std::string& name, LiteralValue value);
	void define(int index, const LiteralValue& value);
	void assign(const Token& name, const LiteralValue& value);
	LiteralValue get(const Token& name);
	std::shared_ptr<Environment> enclosing = nullptr;  // for global scope
	LiteralValue getAt(int distance, int index);
	void assignAt(int distance, int index, LiteralValue value);

	void push_slots(LiteralValue value);

private:
	
	std::unordered_map<std::string, LiteralValue> values;	// for global lookup
	std::vector<LiteralValue> slots;	// for local lookup, indexed by Resolver's "sticky notes"
};

#endif // !ENVIRONMENT_H
