#pragma once

#include "Actions.h"

namespace dust {
	namespace parse {

		template <typename Rule>
		struct control : normal<Rule> {
			// Called before attempting to match Rule
			template <typename Input, typename... States>
			static void start(const Input& in, States&&... ss) {
				normal<Rule>::start(in, ss...);
			}

			// Called if Rule succeeded (input was consumed)
			template <typename Input, typename... States>
			static void success(const Input& in, States&&... ss) {
				normal<Rule>::start(in, ss...);
			}

			// Called if Rule failed (input not consumed)
			template <typename Input, typename... States>
			static void failure(const Input& in, States&&... ss) {
				normal<Rule>::start(in, ss...);
			}

			// Called to raise an exception (for must<Rule>)
			template <typename Input, typename... States>
			static void raise(const Input& in, States&&... ss) {
				normal<Rule>::raise(in, ss...);
			}

			// Called before Rule::match (can make some changes here)
			template <apply_mode A, template <typename...> class Action,
				template <typename...> class Control, typename Input, typename... States>
			static bool match(Input& in, States&&... ss) {
				return normal<Rule>::template match<A, Action, Control>(in, ss...);
			}
		};

	}
}