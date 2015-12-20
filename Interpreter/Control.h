#pragma once

#include "Actions.h"

namespace dust {
	namespace parse {

		//template <typename Rule, typename Input, typename... States>
		//void success(const Input& in, States&&... ss) {
			//normal<Rule>::success(in, ss...);
		//}

		//template <typename Input, typename... States>
		//void success<bad_decimal>(const Input& in, States&&... ss) {
			//normal<bad_decimal>::raise(in, ss...);
		//}

		// Variable templates not yet supported
		//template <typename Rule>
		//static const auto success = normal<Rule>::success;

		//template <>
		//static const auto success<bad_decimal> = normal<bad_decimal>::raise;

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
				//success<Rule>(in, ss...);
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

		//*/
		template <> struct control<bad_decimal> {
			// Called before attempting to match Rule
			template <typename Input, typename... States>
			static void start(const Input& in, States&&... ss) {
				normal<bad_decimal>::start(in, ss...);
			}

			// Called if Rule succeeded (input was consumed)
			template <typename Input, typename... States>
			static void success(const Input& in, States&&... ss) {
				normal<bad_decimal>::raise(in, ss...);
			}

			// Called if Rule failed (input not consumed)
			template <typename Input, typename... States>
			static void failure(const Input& in, States&&... ss) {
				normal<bad_decimal>::start(in, ss...);
			}

			// Called to raise an exception (for must<Rule>)
			template <typename Input, typename... States>
			static void raise(const Input& in, States&&... ss) {
				normal<bad_decimal>::raise(in, ss...);
			}

			// Called before Rule::match (can make some changes here)
			template <apply_mode A, template <typename...> class Action,
				template <typename...> class Control, typename Input, typename... States>
			static bool match(Input& in, States&&... ss) {
				return normal<bad_decimal>::template match<A, Action, Control>(in, ss...);
			}

		};
		//*/

	}
}