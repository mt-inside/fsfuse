<?xml version="1.0" encoding="UTF-8"?>

<!--
    Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.

    Config xml to code generator.
 -->

<xsl:stylesheet
    version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml">

    <xsl:include href="config_common.xsl" />

    <xsl:variable name="header">
        <xsl:text><![CDATA[/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Configuration system definitions.
 *
 * WARNING: THIS FILE IS AUTO-GENERATED. DO NOT ALTER!
 */

#include <stdlib.h>
#include <string.h>

#include "config/config_internal.h"
#include "config_declare.h"
]]></xsl:text>
    </xsl:variable>


    <xsl:template match="/items">
        <xsl:value-of select="$header"/>
        <xsl:value-of select="$newlineX2"/>


        <xsl:text>config_data_t *config_defaults_get( void )</xsl:text>
        <xsl:value-of select="$newline"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$newline"/>
        <xsl:value-of select="$indent"/>
        <xsl:text>config_data_t *defaults = malloc( sizeof(*defaults) );</xsl:text>
        <xsl:value-of select="$newlineX2"/>

        <xsl:apply-templates select="item"/>

        <xsl:value-of select="$newline"/>
        <xsl:value-of select="$indent"/>
        <xsl:text>return defaults;</xsl:text>
        <xsl:value-of select="$newline"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$newline"/>
    </xsl:template>

    <xsl:template match="item">
        <xsl:value-of select="$indent"/>
        <xsl:text>defaults-></xsl:text>
        <xsl:value-of select="symbol"/>
        <xsl:text> = </xsl:text>
        <xsl:if test="type = 'string'"><xsl:text>strdup( &quot;</xsl:text></xsl:if>
        <xsl:value-of select="default"/>
        <xsl:if test="type = 'string'"><xsl:text>&quot; )</xsl:text></xsl:if>
        <xsl:value-of select="$sc"/>
        <xsl:value-of select="$newline"/>

        <!-- All config items are present in the defaults -->
        <xsl:value-of select="$indent"/>
        <xsl:text>defaults-></xsl:text>
        <xsl:value-of select="symbol"/>
        <xsl:text>_present</xsl:text>
        <xsl:text> = 1</xsl:text>
        <xsl:value-of select="$sc"/>
        <xsl:value-of select="$newline"/>
    </xsl:template>

</xsl:stylesheet>
