#
#       © Christian Sommerfeldt Øien
#       All rights reserved
"""Parametric Context-Sensitive Bracketed Lindenmayer System"""

from re import compile as COMPILE, sub as SUB
leftcat = COMPILE(r"(?=\S)([^_a-zA-Z])")
rightcat = COMPILE(r"([^_a-zA-Z])(?=\S)")
from sys import stderr
from random import random as rnd_1, seed as rnd_seed

derive_last_n = 0  #nonzero to tweek to skip all-but-last-n on any (on-branch) axiom
eval_globals = {}

class Settings:
    @staticmethod
    def only_last_nodes(n):
        global derive_last_n
        derive_last_n = n
    @staticmethod
    def random_seed(n):
        rnd_seed(n)
    @staticmethod
    def param_globals(d):
        global eval_globals
        eval_globals = d

def parameters_zip(expr, par):
    p = par.replace(' ', '').split(',')  #?replace space needed?
    e = expr.replace(' ', '').split(',')  #?replace space needed?
    if len(p) != len(e):
        return None
    r = []
    for i in range(len(p)):
        r.append((p[i], e[i]))
    return r


def parameters_eval(assignments, expr):
    g = dict(eval_globals)
    e = expr
    for (n, v) in assignments:
        e = SUB(r'\b%s\b' % (n,), v, e)
    v = eval(e, g)
    return [v] if type(v) != tuple else v


def parameters_match(assignments, expr):
    if not expr: return True
    v = parameters_eval(assignments, expr)
    for t in v:
        if not t:
            return False
    return True


def parameters_apply(assignments, expr):
    if expr: return ",".join([str(s) for s in parameters_eval(assignments, expr)])


def tokenize(e):
        e = SUB(leftcat, r" \1", e)
        e = SUB(rightcat, r"\1 ", e)
        return e.split()


class Scanner:
    def __init__(self, e):
        self.tokens = tokenize(e)
        self.n = len(self.tokens)
        self.i = 0
        self.lexeme = None
        self.halt(None)

    def halt(self, at):
        self.halted_at = at

    def halted(self):
        return self.halted_at

    def token(self):
        return None if self.i >= self.n else self.tokens[self.i]

    def more(self):
        return self.lexeme or self.token()

    def skip_token(self):
        self.i += 1

    def peek_lexeme(self):
        if self.lexeme is None:
            if self.more():
                t = self.token()
                self.skip_token()
                if t == "'":
                    self.lexeme = ""
                    while self.more():
                        t = self.token()
                        self.skip_token()
                        if t == "'":
                            break
                        self.lexeme += t
                elif t == "]" and self.token() == "[":
                    self.lexeme = "]["
                    self.skip_token()
                elif t == "}" and self.token() == "{":
                    self.lexeme = "}{"
                    self.skip_token()
                else:
                    self.lexeme = t
        self.debug("peek_lexeme --> %s" % self.lexeme)
        return self.lexeme

    def get_lexeme(self):
        lexeme = self.peek_lexeme()
        self.debug("get_lexeme ---> %s" % self.lexeme)
        self.lexeme = None
        return lexeme

    def debug(self, s):
        #print s
        pass


def parameters_parse(scanner):
    assert scanner.get_lexeme() == ":", "not at parameters"
    return scanner.get_lexeme()


class Node:
    def abstract(self): assert False
    __init__ = abstract
    __str__  = abstract
    visit_by = abstract


class Symbol(Node):
    def __init__(self, letter):
        for c in ":()[]{}":
            assert c not in letter
        self.letter = letter
        self.parameters = None

    def __str__(self):
        if self.parameters is None: return self.letter
        else: return "%s:'%s'" % (self.letter, self.parameters)

    def copy(self):
        s = Symbol(self.letter)
        s.parameters = self.parameters
        #note: shallow copy is fine
        return s

    def instantiate(self, assignments):
        self.parameters = parameters_apply(assignments, self.parameters)

    def parse_parameters(self, scanner):
        if scanner.peek_lexeme() == ":":
            self.parameters = parameters_parse(scanner)

    def visit_by(self, visitor):
        visitor.symbol(self)


def symbol_matches(n, m, assignments):
    if n.letter != m.letter: return False
    if n.parameters is None and m.parameters is None: return True
    if n.parameters is None or m.parameters is None: return False
    r = parameters_zip(m.parameters, n.parameters)
    if r is None: return False
    assignments.extend(r)
    return True


def parse_symbol(scanner):
    s = Symbol(scanner.get_lexeme())
    s.parse_parameters(scanner)
    return s


class Axiom:
    def __init__(self, fork):
        self.dad = fork
        self.nodes = []
        self.mark(None)

    def mark(self, at):
        assert not at or self.dad is None
        self.marked_at = at

    def marked(self):
        return self.marked_at

    def __str__(self):
        return " ".join([str(n) for n in self.nodes])

    def parse_nodes(self, scanner):
        assert not scanner.halted()
        while scanner.more():
            t = scanner.peek_lexeme()
            if t in ("(", ")", "][", "]", "}{", "}"):
                assert not scanner.halted() or t == ")"
                break
            elif t == "[":
                x = len(self.nodes)
                assert not (x and isinstance(self.nodes[x - 1], Fork))
                f = Fork(self, x)
                f.parse_branches(scanner)
                self.nodes.append(f)
            else:
                self.nodes.append(parse_symbol(scanner))

    def derived_nodes(self, a, rules):
        nodes = []
        nodes.extend(self.nodes[:-derive_last_n])
        x = len(nodes) - 1
        for n in self.nodes[-derive_last_n:]:
            x += 1
            if isinstance(n, Fork):
                assert n.trunk == self
                f = Fork(a, len(nodes))
                f.branches = n.derived_branches(f, rules)
                nodes.append(f)
                continue
            replacement_nodes = None
            for r in rules:
                assignments = []
                if r.matcher.matches(self, x, assignments):
                    replacement_nodes = r.replacement_copy(a, len(nodes), assignments)
                    if replacement_nodes is None:  #it's a CHOP
                        return nodes
                    break
            if replacement_nodes:
                nodes.extend(replacement_nodes)
            else:
                nodes.append(n.copy())
        return nodes

    def on_unique_tail_forks(self):
        f = self.dad
        if not f: return True
        if len(f.branches) != 1: return False
        if len(f.trunk.nodes) == f.index: return False
        return f.trunk.on_unique_tail_forks()

    def visit_by(self, visitor):
        for n in self.nodes:
            n.visit_by(visitor)


class Fork(Node):
    def __init__(self, axiom, index):
        self.trunk = axiom
        self.index = index
        self.branches = []

    def __str__(self):
        return "[" + "][".join([str(a) for a in self.branches]) + "]"

    def parse_branches(self, scanner):
        assert not scanner.halted()
        assert scanner.get_lexeme() == "[", "not at fork"
        while True:
            a = Axiom(self)
            a.parse_nodes(scanner)
            self.branches.append(a)
            t = scanner.peek_lexeme()
            if t == "]":
                assert scanner.get_lexeme() == "]"
                return
            if t == "][":
                assert scanner.get_lexeme() == "]["
                continue
            if not scanner.halted():
                scanner.halt(a)
            break

    def derived_branches(self, f, rules):
        branches = []
        for b in self.branches:
            assert b.dad == self, "inconsistent axiom structure"
            a = Axiom(f)
            a.nodes = b.derived_nodes(a, rules)
            branches.append(a)
        return branches

    def visit_by(self, visitor):
        for b in self.branches:
            visitor.left_bracket()
            b.visit_by(visitor)
            visitor.right_bracket()


def nodes_match(match_nodes, ignore_letter, exhaust, nodes, assignments):
    if len(match_nodes) > len(nodes):
        return False
    i = j = 0
    while i < len(match_nodes) and j < len(nodes):
        n = nodes[j]
        if isinstance(match_nodes[i], Fork):
            if not isinstance(n, Fork): break
            if not branches_match(match_nodes[i].branches, ignore_letter, n.branches, assignments): break
        else:
            if isinstance(n, Fork):
                j += 1
                continue
            elif ignore_letter(n.letter):
                i -= 1
            elif not symbol_matches(match_nodes[i], n, assignments): break
        i += 1
        j += 1
    if i != len(match_nodes):
        return False
    if not exhaust:
        return True
    for k in range(j, len(nodes)):
        n = nodes[k]
        if isinstance(n, Fork):
            continue
        if not ignore_letter(n.letter):
            return False
    return True


def branches_match(match_branches, ignore_letter, branches, assignments):
    if len(match_branches) > len(branches):
        return False
    i = j = 0
    while i < len(match_branches):
        if j >= len(branches):
            break
        b = branches[j]
        if not nodes_match(match_branches[i].nodes, ignore_letter, False, b.nodes, assignments):
            i -= 1
        i += 1
        j += 1
    return i == len(match_branches)


def left_context_matches(left_context, ignore_letter, matched_nodes, assignments, next_dad, as_dad = False):
    if left_context.dad:
        if not next_dad:
            return False
        if not left_context_matches(
               left_context.dad.trunk,
               ignore_letter,
               next_dad.trunk.nodes[:next_dad.index + 1],
               assignments,
               next_dad.trunk.dad,
               True):
            return False
    exhaust = left_context.dad is not None
    context_r = list(reversed(left_context.nodes))
    matched_r = list(reversed(matched_nodes))
    if as_dad:
        assert isinstance(context_r[0], Fork)
        assert isinstance(matched_r[0], Fork)
        return nodes_match(context_r[1:], ignore_letter, exhaust, matched_r[1:], assignments)
    else:
        return nodes_match(context_r, ignore_letter, exhaust, matched_r, assignments)


class Matcher:
    def __init__(self, l_axiom, symbol, r_axiom, context_ignore):
        self.left_context = l_axiom
        self.symbol = symbol
        self.right_context = r_axiom
        self.context_ignore = context_ignore or []
        self.parameters = None

    def ignore_letter(self, letter):
        return is_operation(letter) or letter in self.context_ignore

    def right_matches(self, axiom, i, assignments):
        return nodes_match(self.right_context.nodes, self.ignore_letter, False, axiom.nodes[i + 1:], assignments)

    def left_matches(self, axiom, i, assignments):
        c = self.left_context.marked() or self.left_context
        return left_context_matches(c, self.ignore_letter, axiom.nodes[:i], assignments, axiom.dad)

    def matches(self, axiom, i, assignments):
        n = axiom.nodes[i]
        if isinstance(n, Fork): return False
        elif not symbol_matches(self.symbol, n, assignments): return False
        elif not self.right_matches(axiom, i, assignments): return False
        elif not self.left_matches(axiom, i, assignments):  return False
        elif not parameters_match(assignments, self.parameters): return False
        return True

    def __str__(self):
        m = "(%s) %s (%s)" % (self.left_context, self.symbol, self.right_context)
        return "%s :'%s'" % (m, self.parameters) if self.parameters else m

    def parse_parameters(self, scanner):
        if scanner.peek_lexeme() == ":":
            self.parameters = parameters_parse(scanner)


class ParameterInstantiateVisitor:
    def __init__(self, assignments):
        self.assignments = assignments

    def left_bracket(self): pass
    def right_bracket(self): pass

    def symbol(self, s):
        s.instantiate(self.assignments)


def parse_replacement(scanner):
    if scanner.peek_lexeme() == ".":
        scanner.get_lexeme()
        return None  #chop
    else:
        a = Axiom(None)
        a.parse_nodes(scanner)
        assert not scanner.halted()
        assert not isinstance(a.nodes[0], Fork), "malformed replacement"
        return a


def repr_possible_replacement(probability_expr, replacement):
    return ":'%s' %s" % (probability_expr, replacement or ".")


class Rule:
    def __init__(self, s_given, context_ignore = None):
        if isinstance(s_given, Scanner): scanner = s_given
        else: scanner = Scanner(s_given)
        left, sym, right = self.parse_match_expr(scanner)
        m = Matcher(left, sym, right, context_ignore)
        m.parse_parameters(scanner)
        
        self.matcher = m
        self.replacement_possibilities = []
        self.parse_replacement_expr(scanner)
        if not isinstance(s_given, Scanner):
            assert not scanner.more(), "rule trailer"

    def __str__(self):
        if len(self.replacement_possibilities) == 1:
            return "%s %s" % (self.matcher, repr_possible_replacement(self.replacement_possibilities[0]))
        else:
            return "%s {%s}" % (self.matcher, 
                    "}{".join(repr_possible_replacement(self.replacement_possibilities)))

    def parse_match_expr(self, scanner):
        assert scanner.get_lexeme() == "("
        left = Axiom(None)
        left.parse_nodes(scanner)
        if scanner.halted():
            left.mark(scanner.halted())
            assert left.marked().on_unique_tail_forks(), "malformed left-context"
            scanner.halt(None)
        assert scanner.get_lexeme() == ")"
        sym = parse_symbol(scanner)
        assert scanner.get_lexeme() == "("
        right = Axiom(None)
        right.parse_nodes(scanner)
        assert not scanner.halted()
        assert scanner.get_lexeme() == ")"
        return left, sym, right
    
    def parse_replacement_expr(self, scanner):
        if scanner.peek_lexeme() != "{":
            self.replacement_possibilities.append((None, parse_replacement(scanner)))
        else:
            assert scanner.get_lexeme() == "{"
            while True:
                if scanner.peek_lexeme() == ":":
                    p = parameters_parse(scanner)
                else:
                    p = None
                self.replacement_possibilities.append((p, parse_replacement(scanner)))
                l = scanner.get_lexeme()
                if l == "}":
                    break
                assert l == "}{"

    def pick_possible_replacement(self, assignments):
        if len(self.replacement_possibilities) == 1:
            return self.replacement_possibilities[0][1]
        totals = []
        t = 0
        for (p, r) in self.replacement_possibilities:
            if p: w = parameters_apply(assignments, p)
            else: w = 1
            t += float(w)
            totals.append(t)
        s = t * rnd_1()
        for i in range(len(totals)):
            if s <= totals[i]:
                return self.replacement_possibilities[i][1]
        assert False

    def replacement_copy(self, axiom, index, assignments):
        nodes = []
        i = index
        v = ParameterInstantiateVisitor(assignments)
        r = self.pick_possible_replacement(assignments)
        if r is None:
            return r
        for p in r.nodes:
            if isinstance(p, Fork):
                f = Fork(axiom, i)
                f.branches = p.derived_branches(f, [])
                f.visit_by(v)
                nodes.append(f)
            else:
                s = p.copy()
                s.visit_by(v)
                nodes.append(s)
            i += 1
        return nodes


def is_operation(letter):
    return len(letter) == 1 and letter != "_" and not letter.isalnum()


def numerize(expr):
    return [float(e) for e in expr.split(",")]


class OperationsVisitor:
    def __init__(self, operations, forward = None, forward_invisible = None):
        self.operations = operations
        self.forward = forward
        self.forward_invisible = forward_invisible

    def left_bracket(self): self.operations["["]()
    def right_bracket(self): self.operations["]"]()

    def symbol(self, s):
        letter = s.letter
        if is_operation(letter):
            self.operation(letter, s.parameters)
        else:
            self.letter(letter)

    def operation(self, letter, parameters):
        args = () if not parameters else numerize(parameters)
        self.operations[letter](*args)

    def letter(self, letter):
        if letter[0] != "_":
            if letter[-1] == "_" and self.forward_invisible: self.forward_invisible()
            elif self.forward: self.forward()



class SymbolUseVisitor:
    def __init__(self, symbols_used): self.symbols_used = symbols_used
    def symbol(self, s): assert s.letter in self.symbols_used, s.letter
    def left_bracket(self): pass
    def right_bracket(self): pass


class System:
    def __init__(self, expr, rules = None, symbols_used = None, context_ignore = None):
        s = Scanner(expr)
        self.parse_axiom(s)
        if rules:
            assert not s.more(), "axiom trailer"
            self.rules = rules
            assert not context_ignore, "provide in rules"
        else:
            ci_tokens = context_ignore and tokenize(context_ignore)
            self.rules = []
            while s.more():
                self.rules.append(Rule(s, ci_tokens))
            assert not s.more(), "rules trailer"
        assert len(self.rules), "no rules"
        if symbols_used:
            self.visit_all_axioms_by(SymbolUseVisitor(tokenize(symbols_used)))

    def visit_all_axioms_by(self, visitor):
        self.axiom.visit_by(visitor)
        for r in self.rules:
            for (p, repl) in r.replacement_possibilities:
                if repl: repl.visit_by(visitor)
            if r.matcher.left_context: r.matcher.left_context.visit_by(visitor)
            if r.matcher.right_context: r.matcher.right_context.visit_by(visitor)

    def parse_axiom(self, scanner):
        self.axiom = Axiom(None)
        self.axiom.parse_nodes(scanner)
    
    def __str__(self):
        return str(self.axiom) + "\n" \
                + "\n".join([str(r) for r in self.rules])

    def derive_(self):
        d = Axiom(None)
        d.nodes = self.axiom.derived_nodes(d, self.rules)
        self.axiom = d

    def derive(self, n = None, status = None):
        if n is None:
            self.derive_()
            return
        for i in range(n):
            if status: status(i)
            self.derive_()


if __name__ == "__main__":
    from sys import argv, stdin
    if len(argv) > 1:
        ls = System(stdin.read())
        c = int(argv[1])
        for n in range(1, c+1):
            ls.derive()
            print("%d:  %s" % (n, ls.axiom))
    else:
        print("selftest:", end=' ')
        ls = System("a () a () a [b] (a[) b ([c]) B [d] e [f X [z z][z]] " \
                         "(a[) b () b [c] (B [] e [f) X ([z][z]) Y")
        for n in range(4):
            ls.derive()
        assert str(ls.axiom) == \
            "a [b] [b [c]] [B [d] e [f X [z z][z]] [c]] [B [d] e [f Y [z z][z]] [c]]"

        ls = System("a : '1' a : '2' " \
            "() a:'xx'(a:'x') :'xx<x,xx>0' b:'xx+3'")
        ls.derive()
        assert str(ls.axiom) == "b:'4' a:'2'"

        ls = System("a b [f d] (b[)d() e")
        ls.derive()
        assert str(ls.axiom) == "a b [f d]"
        ls = System("a b [f d] (b[)d() e", context_ignore = "f")
        ls.derive()
        assert str(ls.axiom) == "a b [f e]"

        rules = [Rule("() a:'x' () /:'x'b"), Rule("()d().")]
        ls = System("^:'3,2'a:'1'[/c:'0']d /:'4'e", rules)
        ls.derive()
        r = []
        ls.axiom.visit_by(OperationsVisitor({
            "[": lambda k="[": r.append(k),
            "]": lambda k="]": r.append(k),
            "/": lambda k=0: r.append(k),
            "^": lambda p, q: r.extend([p, q])}))
        assert r == [3, 2, 1, '[', 0, ']']
        print("done")

