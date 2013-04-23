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

    <xsl:template match="/items">
        <xsl:value-of select="$header_h"/>
        <xsl:value-of select="$newlineX2"/>


        <xsl:apply-templates select="item"/>

    </xsl:template>

    <xsl:template match="item">
        <xsl:text>extern </xsl:text>
        <xsl:call-template name="type_to_ctype">
            <xsl:with-param name="type" select="type"/>
        </xsl:call-template>
        <xsl:value-of select="$space"/>
        <xsl:text>config_</xsl:text>
        <xsl:value-of select="symbol"/>
        <xsl:value-of select="$space"/>
        <xsl:text>( config_reader_t *reader )</xsl:text>
        <xsl:value-of select="$sc"/>
        <xsl:value-of select="$newline"/>
    </xsl:template>

</xsl:stylesheet>
