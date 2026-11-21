USE [dnworld]
GO
/****** Object:  View [dbo].[VIX_PartyMemberCount]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[VIX_PartyMemberCount]
WITH SCHEMABINDING AS
SELECT PartyID, COUNT_BIG(*) AS PartyMemberCount, MAX(CharacterTypeCode) AS CharacterTypeCode
FROM dbo.PartyJoins
GROUP BY PartyID;
GO
