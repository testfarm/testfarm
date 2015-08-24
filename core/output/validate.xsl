<?xml version="1.0"?>

<!-- TestFarm Validation State Display

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
