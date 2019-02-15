#pragma once

#include <exception>

namespace pure {
	struct var;
	namespace Interface {
		/**
		 Interface for exceptions. It extends std::exception, but offers more capabilities to retrieve more than just
		 an error message. This can be used in catch clauses to catch non std::exceptions.
		 */
		struct Exception : public std::exception {
			virtual var error_message ();
			virtual int error_code () noexcept;
			virtual var error_type ();
			virtual var error_data ();
		};
	}

	struct operation_not_supported : Interface::Exception {
		const char* what () const noexcept override { return "Requested operation is not supported"; }
	};
}
