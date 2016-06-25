/*

			// Testing function calling and definition
			t.initSubTest("Functions");
				e.push("abs");
				e.push([](EvalState& e) {
					auto x = (int)e;
					e.push(x > 0 ? x : -x);
					return 1;
				});
				e.set(EvalState::SCOPE);

				e.addMember(e.getTS().getType("Int"), "abs", [](EvalState& e) {
					e.enableObjectSyntax().get(EvalState::SELF);

					auto x = (int)e;
					e.push(x > 0 ? x : -x);
					return 1;
				});

				e.push("add");
				e.push([](EvalState& e) {
					e.push((int)e + (int)e);
					return 1;
				});
				e.set(EvalState::SCOPE);

				e.push("give5");
				e.push([](EvalState& e) {
					e.push(5);
					return 1;
				});
				e.set(EvalState::SCOPE);

				e.push("bound");
				e.push([](EvalState& e) {
					auto d = (double)e;
					e.push((int)std::floor(d));
					e.push((int)std::ceil(d));
					return 2;
				});
				e.set(EvalState::SCOPE);

				e.push("max");
				e.push([](EvalState& e) {
					Optional max{ e }, opt;							// Max may have a value, opt guaranteed to be nil

					while (opt.set(e)) {							// For each optional argument, try to beat max
						e.push(max);
						e.push(opt);
						e.callOp("_op>");							// Run opt > max

						if ((bool)e) max.set(opt);
					}

					e.push(max);
					return 1;
				});
				e.set(EvalState::SCOPE);

				e.push("sum");
				e.push([](EvalState& e) {
					Optional sum{ e }, nxt;							// But I want to demonstrate Optional

					while (nxt.copy(e)) {
						e.push(sum);
						e.callOp("_op+");

						sum.set(e);
					}

					e.push(sum);
					return 1;
				});
				e.set(EvalState::SCOPE);

			t.closeSubTest();


*/