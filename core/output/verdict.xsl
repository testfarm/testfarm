<?xml version="1.0"?>

<!-- TestFarm Verdict Display

This file is part of TestFarm,
the Test Automation Tool for Embedded Software.
Please visit http://www.testfarm.org.

TestFarm is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TestFarm is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
-->

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:variable name="verdict-config-file" select="'/opt/testfarm/lib/verdict.xml'"/>

<xsl:variable name="verdict-passed" select="document($verdict-config-file)//VERDICT[@id='PASSED']/@level"/>
<xsl:variable name="verdict-failed" select="document($verdict-config-file)//VERDICT[@id='FAILED']/@level"/>
<xsl:variable name="verdict-inconclusive" select="document($verdict-config-file)//VERDICT[@id='INCONCLUSIVE']/@level"/>
<xsl:variable name="verdict-skip" select="document($verdict-config-file)//VERDICT[@id='SKIP']/@level"/>

<xsl:template name="verdict-summary">
  <xsl:for-each select="document($verdict-config-file)//VERDICT">
    <xsl:sort select="@level"/>
    <xsl:if test="@level>'0'"><xsl:text>, </xsl:text></xsl:if>
    <font color="{@color}"><b><u><xsl:value-of select="substring(@id,1,1)"/></u></b><xsl:value-of select="substring(@id,2)"/></font>
  </xsl:for-each>
</xsl:template>

<xsl:template name="verdict-display">
  <xsl:param name="verdict"/>
  <xsl:variable name="item" select="document($verdict-config-file)//VERDICT[@level=$verdict]"/>
  <font color="{$item/@color}"><xsl:value-of select="$item/@id"/></font>
</xsl:template>

<xsl:template name="verdict-symbol">
  <xsl:param name="verdict"/>
  <xsl:variable name="item" select="document($verdict-config-file)//VERDICT[@level=$verdict]"/>
  <xsl:variable name="id" select="$item/@id"/>
  <xsl:choose>
    <xsl:when test="$id!=''">
      <b style="color:{$item/@color};"><xsl:value-of select="substring($id,1,1)"/></b>
    </xsl:when>
    <xsl:otherwise>
      -
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

</xsl:stylesheet>
