<?xml version="1.0"?>

<!-- TestFarm Validation State Display -->
<!-- $Revision: 127 $ -->
<!-- $Date: 2006-06-29 12:25:41 +0200 (jeu., 29 juin 2006) $ -->

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:param name="validate-config" select="'/var/testfarm/lib/validate.xml'"/>

<xsl:variable name="validate-config-file">
  <xsl:choose>
    <xsl:when test="document($validate-config)/VALIDATE_CONFIG">
      <xsl:value-of select="$validate-config"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="'/opt/testfarm/lib/validate.xml'"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:variable>


<xsl:variable name="validate-max" select="document($validate-config-file)//VALIDATE[last()]/@level"/>

<xsl:template name="validate-display">
  <xsl:param name="level"/>
  <xsl:param name="info"/>
  <xsl:variable name="item" select="document($validate-config-file)//VALIDATE[@level=$level]"/>
  <i style="color:{$item/@color};"><xsl:value-of select="$item/@id"/>
  <xsl:if test="$info">, <xsl:value-of select="$info"/></xsl:if>
  </i>
</xsl:template>

</xsl:stylesheet>
