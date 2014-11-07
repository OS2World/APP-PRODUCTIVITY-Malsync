/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef __MAL31DBCONFIG_H__
#define __MAL31DBCONFIG_H__

/* The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Mobile Application Link.
 *
 * The Initial Developer of the Original Code is AvantGo, Inc.
 * Portions created by AvantGo, Inc. are Copyright (C) 1997-1999
 * AvantGo, Inc. All Rights Reserved.
 *
 * Contributor(s):
 */


#include <AGDBConfig.h>
#include <AGReader.h>
#include <AGWriter.h>

void MAL31DBConfigReadData(AGDBConfig *dbconfig, AGReader *r);
void MAL31DBConfigWriteData(AGDBConfig *dbconfig, AGWriter *w);

#endif
