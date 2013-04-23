<?xml version="1.0" encoding="UTF-8"?>

<!--
    Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.

    Config xml to code generator.
 -->

<xsl:stylesheet
    version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml">

    <xsl:output method="text" encoding="ascii"/>


    <xsl:variable name="header_h">
        <xsl:text><![CDATA[/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Configuration system declarations.
 *
 * WARNING: THIS FILE IS AUTO-GENERATED. DO NOT ALTER!
 */
]]></xsl:text>
    </xsl:variable>

    <xsl:variable name="header_c">
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

#include "config_reader.h"
#include "config_internal.h"
]]></xsl:text>
    </xsl:variable>


    <xsl:variable name="space">
        <xsl:text> </xsl:text>
    </xsl:variable>

    <xsl:variable name="comma">
        <xsl:text>,</xsl:text>
    </xsl:variable>

    <xsl:variable name="sc">
        <xsl:text>;</xsl:text>
    </xsl:variable>

    <xsl:variable name="indent">
        <xsl:text>    </xsl:text>
    </xsl:variable>

    <xsl:variable name="newline">
        <xsl:text>
</xsl:text>
    </xsl:variable>

    <xsl:variable name="newlineX2">
        <xsl:value-of select="$newline"/>
        <xsl:value-of select="$newline"/>
    </xsl:variable>


    <xsl:variable name="config_type_name">
        <xsl:text>config_data_t</xsl:text>
    </xsl:variable>

    <xsl:template name="type_to_enum">
        <xsl:param name="type"/>

        <xsl:choose>
            <xsl:when test="$type = 'integer'">
                <xsl:text>config_item_type_INTEGER</xsl:text>
            </xsl:when>
            <xsl:when test="$type = 'float'">
                <xsl:text>config_item_type_FLOAT</xsl:text>
            </xsl:when>
            <xsl:when test="$type = 'string'">
                <xsl:text>config_item_type_STRING</xsl:text>
            </xsl:when>
            <xsl:when test="$type = 'string_collection'">
                <xsl:text>config_item_type_STRING_COLLECTION</xsl:text>
            </xsl:when>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="type_to_ctype">
        <xsl:param name="type"/>

        <xsl:choose>
            <xsl:when test="$type = 'integer'">
                <xsl:text>int</xsl:text>
            </xsl:when>
            <xsl:when test="$type = 'float'">
                <xsl:text>double</xsl:text>
            </xsl:when>
            <xsl:when test="$type = 'string'">
                <xsl:text>char *</xsl:text>
            </xsl:when>
            <xsl:when test="$type = 'string_collection'">
                <xsl:text>char **</xsl:text>
            </xsl:when>
        </xsl:choose>
    </xsl:template>
    
</xsl:stylesheet>
