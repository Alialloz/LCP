#!/usr/bin/env bats

bats_require_minimum_version 1.5.0

setup() {
	mkdir -p t/a t/b
	echo "AAAAAA" > t/a/a
	echo "AAAAAA" > t/b/a
	echo "BBBBBB" > t/a/b
	ln -sf lien t/a/l
}

teardown() {
	rm -rf t/  # commentez pour garder les artefacts de tests
	true
}

# Lancer les tests avec `BATS_VALGRIND` active la detection des fuites
if [[ -n "$BATS_VALGRIND" ]]; then
	eval orig_"$(declare -f run)"
	run() {
		orig_run valgrind -q --leak-check=full "$@"
	}
fi

trun() {
	run ./inject "$@" 3>t/trace.log
	trace=`cat t/trace.log`
	trace=`echo $trace`
}

tsrun() {
	run --separate-stderr ./inject "$@" 3>t/trace.log
	trace=`cat t/trace.log`
	trace=`echo $trace`
}

trace_run() {
	run --separate-stderr strace "$@"
}

@test "usage: -j <= 1 devrait retourner une erreur" {
	tsrun ./lcp -j 1 t/a/a t/b/a
	[ "$status" -ne 0 ]
	[ "$stderr" != "" ]
}

@test "Ne devrait pas copier de fichiers non-reguliers" {
	trun ./lcp /dev/zero t/b/
	[ "$status" -ne 0 ]
}

@test "Ne devrait rien copier si un fichier est invalide" {
	trun ./lcp t/a/b t/a/XXXX t/b/
	[ "$status" -ne 0 ]
	[ ! -f t/b/b ]
}

@test "-l devrait copier le lien" {
	trun ./lcp -l t/a/l t/b/l
	readlink t/b/l
	content1=`readlink t/a/l`
	content2=`readlink t/b/l`
	[ "$content1" = "$content2" ]
	[ "$status" -eq 0 ]
}

@test "-j 2 devrait appeler fork" {
	TRACEALL=1 trun ./lcp -j 2 t/a/a t/b/
	[ "$status" -eq 0 ]
	[[ "$trace" =~ fork ]]
}

@test "-j 3 devrait appeler fork une seule fois si un seul fichier source" {
	TRACEALL=1 trun ./lcp -j 3 t/a/a t/b/
	[ "$status" -eq 0 ]
	[[ $trace =~ (.*fork.*){1} ]]
}

@test "-j 3 devrait appeler fork deux fois si deux fichiers sources" {
	TRACEALL=1 trun ./lcp -j 3 t/a/a t/a/a t/b/
	[ "$status" -eq 0 ]
	[[ $trace =~ (.*fork.*){2} ]]
}

@test "-j 2 devrait appeler fork trois fois si trois fichiers sources" {
	TRACEALL=1 trun ./lcp -j 2 t/a/a t/a/a t/a/a t/b/
	[ "$status" -eq 0 ]
	[[ $trace =~ (.*fork.*){3} ]]
}

@test "-j 10 devrait appeler fork qu'une seule fois par source" {
	TRACEALL=1 trun ./lcp -j 10 t/a/a t/a/a t/a/a t/b/
	[ "$status" -eq 0 ]
	[[ $trace =~ (.*fork.*){3} ]]
}

@test "-j 2 devrait appeler wait pour le fork" {
	trace_run -f ./lcp -j 2 t/a/a t/b
	waits=`echo "$stderr" | grep -v "resumed" | grep -o "wait" | wc -l`
	[[ waits -eq 1 ]]
}

@test "-j 3 devrait appeler wait pour chaque fork" {
	trace_run -f ./lcp -j 3 t/a/a t/a/a t/b
	waits=`echo "$stderr" | grep -v "resumed" | grep -o "wait" | wc -l`
	[[ waits -eq 2 ]]
}

@test "-j 2 devrait appeler wait pour chaque fork" {
	TRACEALL=1 trun ./lcp -j 2 t/a/a t/a/a t/b/
	[[ $trace =~ (.*fork.*wait){2} ]]
}

@test "-j 3 devrait appeler fork deux fois avant de wait" {
	TRACEALL=1 trun ./lcp -j 3 t/a/a t/a/a t/a/a t/b/
	[[ $trace =~ (.*fork){2}.*wait.*fork(.*wait){2} ]]
}
