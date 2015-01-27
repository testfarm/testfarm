<?xml version="1.0"?>

<!-- TestFarm Report default CSS -->
<!-- $Revision: 281 $ -->
<!-- $Date: 2006-11-16 10:56:08 +0100 (jeu., 16 nov. 2006) $ -->

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:param name="styles-custom-file" select="'/var/testfarm/lib/styles.xml'"/>
<xsl:param name="styles-standard-file" select="'/opt/testfarm/lib/styles.xml'"/>

<xsl:variable name="styles-file">
  <xsl:choose>
    <xsl:when test="document($styles-custom-file)">
      <xsl:value-of select="$styles-custom-file"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$styles-standard-file"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:variable>

<xsl:template name="styles">
  <xsl:copy-of select="document($styles-file)"/>
</xsl:template>

</xsl:stylesheet>
