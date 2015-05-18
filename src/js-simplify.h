/* Copyright 2015 9x6.me. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Notis Hell (notishell@gmail.com)
 */
#ifndef SRC_ALI_UA_REVERSE_H_
#define SRC_ALI_UA_REVERSE_H_

/**
 * @file
 * @mainpage js-simplify 
 *
 * This is method to simplify js
 *
 * Example:
 * @code
 *    char *old_js = "(function(){var f9=((0x3f3|01116)%93);})();";
 *    char *new_js = (char *) malloc(strlen(ua) + 1);
 *    int size = js_simplify(old_js, new_js);
 *    if (size >= 0) {
 *    	printf("result: %s", new_js);
 *    } else {
 *    	printf("failed!");
 *    }
 * @endcode
 */
int js_simplify(char *input, char *output);


#endif /* SRC_ALI_UA_REVERSE_H_ */
