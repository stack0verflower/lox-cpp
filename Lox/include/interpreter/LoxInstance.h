#ifndef LOXINSTANCE_H
#define LOXINSTANCE_H

/*
class Person {
  sayName() {
	print this.name;
  }
}

var jane = Person();
jane.name = "Jane";

var bill = Person();
bill.name = "Bill";

bill.sayName = jane.sayName;
bill.sayName(); // ?

What happens?

So, you are telling me,
bill.sayName = jane.sayName, it first looks up jane.sayName in fields, if it does not get it, it looks in method, yeah a method => Bind and return. 
bill.sayName, this is being assigned, so nonetheless, always creates a field value. 
Then wile accessing, it access field value instead of method (kinda precedence?). 
So, in one way or other, that bounded method of bill.sayName is kinda overshadowed forever, like recessive genes, these will never be called? 

Yeah, Never!!

So, main point of 12.5 is => Fields shadow methods, and binding happens at access time.
Function = free agent. Method = bound to an instance, carries this with it.
*/

#include "interpreter/LoxCallable.h"
#include <memory>
#include <unordered_map>

class LoxInstance : public std::enable_shared_from_this<LoxInstance> {
public:
	LoxInstance(std::shared_ptr<LoxClass> klass);
	LiteralValue get(Token name);
	void set(Token name, LiteralValue value);
	std::string toString();

private:
	std::shared_ptr<LoxClass> klass;
	std::unordered_map<std::string, LiteralValue> fields;
};

#endif // !LOXINSTANCE_H
