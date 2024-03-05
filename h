[33mcommit 016e09a61f15f195c1988de6ddb60739bc1d4e54[m[33m ([m[1;36mHEAD -> [m[1;32mmaster[m[33m, [m[1;31morigin/master[m[33m, [m[1;31morigin/HEAD[m[33m)[m
Author: cgodecke <cgodecke.student.42wolfsburg.de>
Date:   Wed Feb 28 14:17:12 2024 +0100

    config-parser: print now also config file from offset in isRedundantToken

[33mcommit f727d42533b354abef8759d501df3108af0bc6c9[m
Author: jsprenge <jsprenge@student.42wolfsburg.de>
Date:   Wed Feb 28 13:54:52 2024 +0100

    project: Refactor findRoute-related code

[33mcommit 9f04792a4872f59a7956ca8ed077e641caed3e30[m
Author: jsprenge <jsprenge@student.42wolfsburg.de>
Date:   Wed Feb 28 13:51:36 2024 +0100

    slice: Modify splitStart
    
    - Modify behavior to exclude delimiter
    - Add overload for slices as delimiter

[33mcommit 999d0390938991abd59572dd46c3ac07fd50297f[m[33m ([m[1;31morigin/Upload-body-parser[m[33m)[m
Author: jsprenge <jnspr@outlook.de>
Date:   Tue Feb 27 17:29:30 2024 +0100

    debug_utility: Add printRequest function

[33mcommit 415b6e3d8dc495357317106fff5cab33423af465[m
Author: jsprenge <jnspr@outlook.de>
Date:   Tue Feb 27 14:47:04 2024 +0100

    debug_utility: Move printConfig to Debug namespace

[33mcommit eba035a5371f990fbb97b15b12d6e60601386e0c[m
Author: jsprenge <jnspr@outlook.de>
Date:   Tue Feb 27 14:45:01 2024 +0100

    debug_utility: Move printConfig to debug_utility

[33mcommit 7c7848a6b4bfa720449718bd0957d42503a5eea3[m
Author: jsprenge <jnspr@outlook.de>
Date:   Tue Feb 27 03:19:11 2024 +0100

    config_parser: Remove legacy printConfig function

[33mcommit 599aafcaf60f565d7fa1c9043ac2bb256e569ca1[m
Author: jsprenge <jnspr@outlook.de>
Date:   Tue Feb 27 03:15:06 2024 +0100

    config_parser: Unify exceptions
    
    Both TokenizerException and ParserException are now unified as ConfigException.

[33mcommit 8d05458cdcaa5e866fde165c1f334b5e0370d1fe[m
Author: jsprenge <jnspr@outlook.de>
Date:   Tue Feb 27 02:38:41 2024 +0100

    http_request: Add stub parser code

[33mcommit eff9e1694907a509c962bcda99a0f7163b5e0b32[m
Author: jsprenge <jnspr@outlook.de>
Date:   Tue Feb 27 02:31:38 2024 +0100

    example: Change config and add static website

[33mcommit 8d378e14d4a7de83147c17c2b8d78022790550b7[m
Author: jsprenge <jnspr@outlook.de>
Date:   Tue Feb 27 02:13:59 2024 +0100

    project: Move example config to separate directory

[33mcommit 55dc94a5892725a67895e15020bd00552fc121d3[m
Author: jsprenge <jnspr@outlook.de>
Date:   Tue Feb 27 02:10:59 2024 +0100

    main: Add argument handling
    
    Also removes the example configuration and the use of printConfig.

[33mcommit 18e2ba93e3b6cd5d3b37742f18db9074dae69ac8[m
Author: jsprenge <jnspr@outlook.de>
Date:   Tue Feb 27 02:04:18 2024 +0100

    config_parser: Adjust to project style
    
    - Changed file names from UpperCamelCase to snake_case
    - Removed headers from old repository
    - Sorted includes

[33mcommit fcf943ce7f2c3d9f8477acd56cd4c7da2bca0c96[m
Merge: b6bbbeb cfc47cc
Author: fvon-nag <fvon-nag@student.42wolfsburg.de>
Date:   Mon Feb 26 13:06:26 2024 +0100

    Merge branch 'handleRequest'

[33mcommit cfc47cc22bb3b60c2ccb4308c03727f41f1a3241[m[33m ([m[1;31morigin/handleRequest[m[33m)[m
Author: fvon-nag <fvon-nag@student.42wolfsburg.de>
Date:   Mon Feb 26 12:43:25 2024 +0100

    http request  -  fixed small errors

[33mcommit 795c153efabbb2cc033c90c3cf9211517c066fca[m
Author: fvon-nag <fvon-nag@student.42wolfsburg.de>
Date:   Mon Feb 26 12:39:53 2024 +0100

    http request  -  findroute finished

[33mcommit f119ef10276f6475b7382f53fca02d05d1fc11b8[m
Author: fvon-nag <fvon-nag@student.42wolfsburg.de>
Date:   Mon Feb 26 11:19:45 2024 +0100

    added other slice functions

[33mcommit 960acfd24e257e8acd763329c3a8e8c56e4ddb06[m
Author: fvon-nag <fvon-nag@student.42wolfsburg.de>
Date:   Fri Feb 23 17:04:37 2024 +0100

    http request  - findroute attempt finished, need to rework in c++98 compatible way

[33mcommit d24b1f33d8fc3f9aa74a3be75c880b8f6724789d[m
Author: fvon-nag <fvon-nag@student.42wolfsburg.de>
Date:   Fri Feb 23 13:49:27 2024 +0100

      http request  - implemented cast for slice to string proceeded implementing findroute

[33mcommit 78af13aff9fef5082ddfbd853999e99901578916[m
Author: fvon-nag <fvon-nag@student.42wolfsburg.de>
Date:   Fri Feb 23 11:34:24 2024 +0100

    http request - added additional slicing function started with routing, does not compile most likely due to missing or wrong definitions in http_request

[33mcommit b6bbbebf038075e55d3c73f22830fe9907778976[m
Author: jsprenge <jsprenge@student.42wolfsburg.de>
Date:   Thu Feb 22 18:53:21 2024 +0100

    makefile: Define CXX to c++ as per subject

[33mcommit 107725512a7e8b247f67aca1d111e3ffade0068f[m
Merge: 9aacb76 a09d0cd
Author: cgodecke <cgodecke.student.42wolfsburg.de>
Date:   Thu Feb 22 18:44:57 2024 +0100

    Merge branch 'master' of github.com:jnspr/webs3rv

[33mcommit 9aacb768799188e21cb6bc3f53b2fea195198bd0[m
Author: cgodecke <cgodecke.student.42wolfsburg.de>
Date:   Thu Feb 22 18:41:17 2024 +0100

    added the config-parser

[33mcommit a09d0cdd66292127f8a962660a81898273510b7d[m
Author: jsprenge <jsprenge@student.42wolfsburg.de>
Date:   Thu Feb 22 15:42:50 2024 +0100

    http_client: Prepare usage of HttpRequest::Parser

[33mcommit a3104d7228d49d2cee4fd23dfc3acea88aca231a[m
Author: jsprenge <jsprenge@student.42wolfsburg.de>
Date:   Thu Feb 22 15:34:37 2024 +0100

    http_request: Add HTTP request struct and parser

[33mcommit b5c10634239067f168f90de42b49533b1bd07b97[m
Author: jsprenge <jsprenge@student.42wolfsburg.de>
Date:   Wed Feb 21 15:35:31 2024 +0100

    project: Add refactored code

[33mcommit 73a4061ac5e012ed45c73f0de1e7910654000095[m
Author: jsprenge <jsprenge@student.42wolfsburg.de>
Date:   Wed Feb 21 15:35:05 2024 +0100

    project: Add .gitignore
