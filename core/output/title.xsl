<?xml version="1.0"?>

<!-- TestFarm Report Title definition -->
<!-- $Revision: 281 $ -->
<!-- $Date: 2006-11-16 10:56:08 +0100 (jeu., 16 nov. 2006) $ -->

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:param name="title-custom-file" select="'/var/testfarm/lib/title.xml'"/>
<xsl:param name="title-standard-file" select="'/opt/testfarm/lib/title.xml'"/>

<xsl:variable name="title-file">
  <xsl:choose>
    <xsl:when test="document($title-custom-file)">
      <xsl:value-of select="$title-custom-file"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$title-standard-file"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:variable>

<xsl:template name="title">
  <xsl:copy-of select="document($title-file)"/>
</xsl:template>

</xsl:stylesheet>
