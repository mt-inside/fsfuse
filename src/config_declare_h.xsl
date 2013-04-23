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

    <xsl:variable name="guards_top">
        <xsl:text><![CDATA[#ifndef _INCLUDED_CONFIG_DECLARE_H
#define _INCLUDED_CONFIG_DECLARE_H
]]></xsl:text>
    </xsl:variable>

    <xsl:variable name="guards_bottom">
        <xsl:text><![CDATA[
#endif /* _INCLUDED_CONFIG_DECLARE_H */
]]></xsl:text>
    </xsl:variable>


    <xsl:template match="/items">
        <xsl:value-of select="$header_h"/>
        <xsl:value-of select="$guards_top"/>
        <xsl:value-of select="$newlineX2"/>


        <xsl:text>typedef struct</xsl:text>
        <xsl:value-of select="$newline"/>
        <xsl:text>{</xsl:text>
        <xsl:value-of select="$newline"/>

        <xsl:apply-templates select="item"/>

        <xsl:text>} config_data_t;</xsl:text>
        <xsl:value-of select="$newline"/>
        <xsl:value-of select="$guards_bottom"/>
    </xsl:template>

    <xsl:template match="item">
        <xsl:value-of select="$indent"/>
        <xsl:call-template name="type_to_ctype">
            <xsl:with-param name="type" select="type"/>
        </xsl:call-template>
        <xsl:value-of select="$space"/>
        <xsl:value-of select="symbol"/>
        <xsl:value-of select="$sc"/>
        <xsl:value-of select="$newline"/>
    </xsl:template>

</xsl:stylesheet>
