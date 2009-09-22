<?xml version="1.0" encoding="UTF-8"?>

<!--
    Config xml to code generator.

    Copyright (C) Matthew Turner 2009. All rights reserved.

    $Id$
 -->

<xsl:stylesheet
    version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml">

    <xsl:output method="text" encoding="ascii"/>

    <xsl:param name="filetype"/>


    <xsl:variable name="space">
        <xsl:text> </xsl:text>
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


    <xsl:variable name="header_c">
        <xsl:text><![CDATA[/*
 * Configuration system definitions.
 *
 * Copyright (C) Matthew Turner 2009. All rights reserved.
 *
 * $Id$
 *
 * WARNING: THIS FILE IS AUTO-GENERATED. DO NOT ALTER!
 */

#include <stdlib.h>

#include "config_internal.h"
]]></xsl:text>
    </xsl:variable>

    <xsl:variable name="header_h">
        <xsl:text><![CDATA[/*
 * Configuration system declarations.
 *
 * Copyright (C) Matthew Turner 2009. All rights reserved.
 *
 * $Id$
 *
 * WARNING: THIS FILE IS AUTO-GENERATED. DO NOT ALTER!
 */]]></xsl:text>
    </xsl:variable>


    <xsl:template match="/items">
        <xsl:choose>
            <xsl:when test="$filetype = 'c'">
                <xsl:value-of select="$header_c"/>
            </xsl:when>
            <xsl:when test="$filetype = 'h'">
                <xsl:value-of select="$header_h"/>
            </xsl:when>
        </xsl:choose>
        <xsl:value-of select="$newlineX2"/>

        <xsl:apply-templates select="item" mode="symbols"/>

        <xsl:choose>
            <xsl:when test="$filetype = 'c'">
                <xsl:value-of select="$newlineX2"/>
                <xsl:text>config_item config_items[] =</xsl:text>
                <xsl:value-of select="$newline"/>
                <xsl:text>{</xsl:text>
                <xsl:value-of select="$newline"/>

                <xsl:apply-templates select="item" mode="items_table"/>
                
                <xsl:value-of select="$indent"/>
                <xsl:text>{ NULL, 0, NULL }</xsl:text>
                <xsl:value-of select="$newline"/>
                <xsl:text>};</xsl:text>
                <xsl:value-of select="$newline"/>
            </xsl:when>
        </xsl:choose>
    </xsl:template>


    <xsl:template match="item" mode="symbols">
        <xsl:choose>
            <xsl:when test="$filetype = 'c'">
                <xsl:call-template name="type_to_ctype">
                    <xsl:with-param name="type" select="type"/>
                </xsl:call-template>
                <xsl:value-of select="$space"/>
                <xsl:text>config_</xsl:text>
                <xsl:value-of select="symbol"/>
                <xsl:value-of select="$space"/>
                <xsl:text>=</xsl:text>
                <xsl:value-of select="$space"/>
                <xsl:if test="type = 'string'"><xsl:text>&quot;</xsl:text></xsl:if>
                <xsl:value-of select="default"/>
                <xsl:if test="type = 'string'"><xsl:text>&quot;</xsl:text></xsl:if>
                <xsl:value-of select="$sc"/>
                <xsl:value-of select="$newline"/>
            </xsl:when>
            <xsl:when test="$filetype = 'h'">
                <xsl:text>extern </xsl:text>
                <xsl:call-template name="type_to_ctype">
                    <xsl:with-param name="type" select="type"/>
                </xsl:call-template>
                <xsl:value-of select="$space"/>
                <xsl:text>config_</xsl:text>
                <xsl:value-of select="symbol"/>
                <xsl:value-of select="$sc"/>
                <xsl:value-of select="$newline"/>
            </xsl:when>
        </xsl:choose>
    </xsl:template>

    <xsl:template match="item" mode="items_table">
        <xsl:value-of select="$indent"/>
        <xsl:text>{ &amp;config_</xsl:text>
        <xsl:value-of select="symbol"/>
        <xsl:text>, </xsl:text>
        <xsl:call-template name="type_to_enum">
            <xsl:with-param name="type" select="type"/>
        </xsl:call-template>
        <xsl:text>, &quot;</xsl:text>
        <xsl:value-of select="xpath"/>
        <xsl:text>&quot; },</xsl:text>
        <xsl:value-of select="$newline"/>
    </xsl:template>

    
    <xsl:template name="type_to_enum">
        <xsl:param name="type"/>

        <xsl:choose>
            <xsl:when test="$type = 'number'">
                <xsl:text>config_item_type_NUMBER</xsl:text>
            </xsl:when>
            <xsl:when test="$type = 'string'">
                <xsl:text>config_item_type_STRING</xsl:text>
            </xsl:when>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="type_to_ctype">
        <xsl:param name="type"/>

        <xsl:choose>
            <xsl:when test="$type = 'number'">
                <xsl:text>int</xsl:text>
            </xsl:when>
            <xsl:when test="$type = 'string'">
                <xsl:text>char *</xsl:text>
            </xsl:when>
        </xsl:choose>
    </xsl:template>

</xsl:stylesheet>
