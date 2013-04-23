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

#include <stddef.h>
#include <stdlib.h>

#include "config_internal.h"
]]></xsl:text>
    </xsl:variable>


    <xsl:template match="/items">
        <xsl:value-of select="$header"/>
        <xsl:value-of select="$newlineX2"/>


        <xsl:text>config_xml_info_item_t config_xml_info[] =</xsl:text>
        <xsl:value-of select="$newline"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$newline"/>

        <xsl:apply-templates select="item"/>
        
        <xsl:value-of select="$indent"/>
        <xsl:text>{ 0, 0, NULL }</xsl:text>
        <xsl:value-of select="$newline"/>
        <xsl:text>};</xsl:text>
        <xsl:value-of select="$newline"/>
    </xsl:template>

    <xsl:template match="item">
        <xsl:value-of select="$indent"/>
        <xsl:text>{ offsetof(config_data_t, </xsl:text>
        <xsl:value-of select="symbol"/>
        <xsl:text>), </xsl:text>
        <xsl:call-template name="type_to_enum">
            <xsl:with-param name="type" select="type"/>
        </xsl:call-template>
        <xsl:text>, &quot;</xsl:text>
        <xsl:value-of select="xpath"/>
        <xsl:text>&quot; },</xsl:text>
        <xsl:value-of select="$newline"/>
    </xsl:template>
    
</xsl:stylesheet>
