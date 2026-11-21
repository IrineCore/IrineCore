USE [dnstaging]
GO
/****** Object:  View [dbo].[V_DimCharacters]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[V_DimCharacters]
AS
SELECT C1.CharacterID, C1.AccountID, C1.CharacterName, C1.WorldID, C2.CharacterClassCode, C2.CharacterIndex, C2.VillageFirstVisitFlag
	, C2.SkillResetFlag, C2.CharacterLevel, C2.CharacterExp, C2.MissionScore, C2.JobCode, C2.Coin, C2.WarehouseCoin, C2.SkillPoint, C2.LastLoginDate
	, C1.CreateDate, C1.DeleteFlag, C3.DeleteDate, C3.CharacterName AS DeleteCharacterName, C3.DeletePrivateIP, C3.DeletePublicIP, C2.AccountLevelCode
	, C2.PCRoomRebirthCoin, C2.RebirthCoin
FROM dbo.TempCharacters1 C1
	INNER JOIN dbo.TempCharacters2 C2 ON C1.CharacterID = C2.CharacterID
	LEFT OUTER JOIN dbo.TempDeletedCharacters C3 ON C1.CharacterID = C3.CharacterID
GO
