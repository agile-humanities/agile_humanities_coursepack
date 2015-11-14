<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xlink="http://www.w3.org/1999/xlink"
  xmlns:foxml="info:fedora/fedora-system:def/foxml#"
  xmlns:cp="http://agilehumanities.ca/cp">

  <xsl:template match="foxml:datastream[@ID='CPDATA']/foxml:datastreamVersion[last()]">
    <xsl:param name="content"/>
    <xsl:param name="prefix">cp.</xsl:param>
    <xsl:param name="suffix"></xsl:param>
    <xsl:apply-templates select="$content/cp:coursepack">
      <xsl:with-param name="prefix" select="$prefix"/>
      <xsl:with-param name="suffix" select="$suffix"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="cp:coursepack">
    <xsl:param name="prefix">cp.</xsl:param>
    <xsl:param name="suffix"></xsl:param>
    <!-- Create fields for the set of selected elements, named according to the 'local-name' and containing the 'text' -->
    <xsl:for-each select="./*">

      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

  </xsl:template>
</xsl:stylesheet>