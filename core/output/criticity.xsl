<?xml version="1.0"?>

<!-- TestFarm Criticity Display Format -->
<!-- $Revision: 127 $ -->
<!-- $Date: 2006-06-29 12:25:41 +0200 (jeu., 29 juin 2006) $ -->

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:param name="criticity-config" select="'/var/testfarm/lib/criticity.xml'"/>

<xsl:variable name="criticity-config-file">
  <xsl:choose>
    <xsl:when test="document($criticity-config)/CRITICITY_CONFIG">
      <xsl:value-of select="$criticity-config"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="'/opt/testfarm/lib/criticity.xml'"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:variable>

<xsl:template name="criticity-summary">
  <xsl:for-each select="document($criticity-config-file)//CRITICITY">
    <xsl:sort select="@level"/>
    <xsl:if test="@level>'1'"><xsl:text>, </xsl:text></xsl:if>
    <font color="{@color}"><b><u><xsl:value-of select="substring(@id,1,1)"/></u></b><xsl:value-of select="substring(@id,2)"/></font>
  </xsl:for-each>
</xsl:template>

<xsl:template name="criticity-symbol">
  <xsl:param name="level"/>
  <xsl:for-each select="document($criticity-config-file)//CRITICITY[@level=$level]">
    <font color="{@color}"><xsl:value-of select="substring(@id,1,1)"/></font>
  </xsl:for-each>
</xsl:template>

</xsl:stylesheet>
