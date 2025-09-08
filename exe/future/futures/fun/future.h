#include "exe/future/fun/combine/par/all.h"
#include "exe/future/fun/combine/par/first.h"

#include "exe/future/fun/combine/seq/and_then.h"
#include "exe/future/fun/combine/seq/flatten.h"
#include "exe/future/fun/combine/seq/flat_map.h"
#include "exe/future/fun/combine/seq/inline.h"
#include "exe/future/fun/combine/seq/map.h"
#include "exe/future/fun/combine/seq/or_else.h"
#include "exe/future/fun/combine/seq/via.h"

#include "exe/future/fun/make/failure.h"
#include "exe/future/fun/make/just.h"
#include "exe/future/fun/make/submit.h"
#include "exe/future/fun/make/value.h"

#include "exe/future/fun/terminate/apply.h"
#include "exe/future/fun/terminate/detach.h"
#include "exe/future/fun/terminate/get.h"
