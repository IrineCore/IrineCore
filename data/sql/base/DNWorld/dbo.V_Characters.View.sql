USE [dnworld]
GO
/****** Object:  View [dbo].[V_Characters]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[V_Characters]
AS
SELECT CH.WorldID
     , CH.CharacterID
	 , CH.AccountID
	 , CH.AccountName
	 , CH.AccountLevelCode
	 , CH.CharacterName
	 , CH.CharacterClassCode
	 , CH.CharacterIndex
 	 , CH.VillageFirstVisitFlag
	 , CH.DefaultBody
	 , CH.DefaultLeg
	 , CH.DefaultHand
	 , CH.DefaultFoot
	 , CH.DeleteFlag
	 , CH.SkillResetFlag
	 , CH.DeleteDate
	 , CHS.CharacterLevel
	 , CHS.CharacterExp
	 , CHS.MissionScore
	 , CHS.JobCode
	 , CHS.HairColor
	 , CHS.EyeColor
	 , CHS.SkinColor
	 , CHS.LastMapID
	 , CHS.LastVillageMapID
	 , CHS.LastDarkLairVillageMapID
	 , CHS.LastVillageGateNumber
	 , CHS.PositionX
	 , CHS.PositionY
	 , CHS.PositionZ
	 , CHS.Rotate
	 , CHS.Coin
	 , CHS.WarehouseCoin
	 , CHS.LastRebirthCoinDate
	 , CHS.RebirthCoin
	 , CHS.PCRoomRebirthCoin
	 , CHS.SkillPoint
	 , CHS.LastFatigueDate
	 , CHS.Fatigue
	 , CHS.PCRoomFatigue
	 , CHS.WeeklyFatigue
	 , CHS.ViewCashEquip1Flag
	 , CHS.ViewCashEquip2Flag
	 , CHS.CheckSumBin
	 , CHS.LastLogoutDate
	 , CHS.LastLoginDate
	 , CHS.EventFatigue
	 , CHS.EventFatigueGainDate
	 , CHS.VIPFatigue
  FROM dbo.Characters CH
      INNER JOIN dbo.CharacterStatus CHS ON CH.CharacterID = CHS.CharacterID
GO
