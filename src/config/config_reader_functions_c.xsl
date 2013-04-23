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
        <xsl:value-of select="$header_c"/>
        <xsl:value-of select="$newlineX2"/>


        <xsl:apply-templates select="item"/>

    </xsl:template>

    <xsl:template match="item">
        <xsl:call-template name="type_to_ctype">
            <xsl:with-param name="type" select="type"/>
        </xsl:call-template>
        <xsl:value-of select="$space"/>
        <xsl:text>config_</xsl:text>
        <xsl:value-of select="symbol"/>
        <xsl:text><![CDATA[ ( config_reader_t *reader )
{
    int i;
    for( i = reader->datas_len - 1; i >= 0; i-- )
    {
        if( reader->datas[i]->]]></xsl:text>
        <xsl:value-of select="symbol"/>
        <xsl:text>_present</xsl:text>
        <xsl:text><![CDATA[ )
        {
]]></xsl:text>
        <xsl:value-of select="$indent"/><xsl:value-of select="$indent"/><xsl:value-of select="$indent"/>
        <xsl:text>return </xsl:text>
        <xsl:if test="type = 'string'"><xsl:text>strdup( </xsl:text></xsl:if>
        <xsl:text>reader->datas[i]-></xsl:text>
        <xsl:value-of select="symbol"/>
        <xsl:if test="type = 'string'"><xsl:text> )</xsl:text></xsl:if>
        <xsl:value-of select="$sc"/>
        <xsl:text><![CDATA[
        }
    }
    assert( 0 );
    return 0;
}
]]></xsl:text>
    </xsl:template>

</xsl:stylesheet>
