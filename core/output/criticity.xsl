<?xml version="1.0"?>

<!-- TestFarm Criticity Display Format

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
