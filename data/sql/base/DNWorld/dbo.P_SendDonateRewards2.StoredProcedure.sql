USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SendDonateRewards2]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-03-30
description : ??? ?? ?? ? ???? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_SendDonateRewards2]
	@dt2BasisDate datetime2(0) = NULL
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @DonateRankers TABLE (
	RankID int identity(1,1) Primary Key,
	CharacterID bigint
);
DECLARE @DonateRewards TABLE (
	RankID int, --??
	RewardType tinyint, --1=????, 2=????
	ItemID1 int NULL, --???ID1
	ItemCount1 tinyint NULL, --??1
	ItemID2 int NULL, --???ID2
	ItemCount2 tinyint NULL, --??2
	ItemID3 int NULL, --???ID3
	ItemCount3 tinyint NULL, --??3	
	ItemID4 int NULL, --???ID4
	ItemCount4 tinyint NULL  --??4
);
DECLARE @DonateMailInfos TABLE (
	RankID int, --??
	RewardType tinyint, --1=????, 2=????
	SubjectID int, --????ID
	ContentID int --????ID
);

DECLARE @i int = 1
	  , @inbCharacterID bigint
      , @intSubjectID int
      , @intContentID int
      , @intGiveItemID1 int
      , @intGiveItemCount1 int
      , @intGiveItemID2 int
      , @intGiveItemCount2 int
      , @intGiveItemID3 int
      , @intGiveItemCount3 int
      , @intGiveItemID4 int
      , @intGiveItemCount4 int
	  , @datBasisDate date = CASE WHEN @dt2BasisDate IS NULL THEN GETDATE() ELSE CAST(@dt2BasisDate as date) END;

DECLARE @intFirstDayFlag int = DATEPART(DAY, @datBasisDate)


DECLARE	@inyMailTypeCode tinyint	-- ???? (0=??, 1=?????)
	  , @vchCharacterIDs varchar(max)	-- ?? ?? ??? ID ??? (??? : ',') (@@inyMailTypeCode=1 ? ???? ??)
	  , @dtsSendDate smalldatetime	-- ?? ?? ?? ??
	  , @nvcSenderName nvarchar(50)	-- ?? ??? ??
	  , @nvcSubject nvarchar(50)	-- ??
	  , @nvcContent nvarchar(300)	-- ??
	  , @inbAttachCoin bigint	-- ?? ?? ??
	  , @dtsMailScheduleExpireDate smalldatetime	-- (optional) ?? ?? ????. ?? ?? ???? ???? ??? ???? ????.
	  , @intItemID1 int	-- (optional)??? ID
	  , @insItemCount1 smallint	-- (optional)??? ??
	  , @insItemDurability1 smallint	-- (optional)???
	  , @intRandomSeed1 int	-- (optional)?? ??
	  , @inyItemLevel1 tinyint	-- (optional)??? ??
	  , @inyItemPotential1 tinyint	-- (optional)??? ???
	  , @bitSoulBoundFlag1 bit	-- (optional)0=??, 1=?????
	  , @inySealCount1 tinyint	-- (optional)?? ???
	  , @inyItemOption1 tinyint	-- (optional)??? ??
	  , @bitEternityFlag1 bit	-- (optional)0=???? ?? ??, 1=?? ???
	  , @intItemLifespan1 int	-- (optional)??? ?? (?)
	  , @intItemID2 int	-- (optional)??? ID
	  , @insItemCount2 smallint	-- (optional)??? ??
	  , @insItemDurability2 smallint	-- (optional)???
	  , @intRandomSeed2 int	-- (optional)?? ??
	  , @inyItemLevel2 tinyint	-- (optional)??? ??
	  , @inyItemPotential2 tinyint	-- (optional)??? ???
	  , @bitSoulBoundFlag2 bit	-- (optional)0=??, 1=?????
	  , @inySealCount2 tinyint	-- (optional)?? ???
	  , @inyItemOption2 tinyint	-- (optional)??? ??
	  , @bitEternityFlag2 bit	-- (optional)0=???? ?? ??, 1=?? ???
	  , @intItemLifespan2 int	-- (optional)??? ?? (?)
	  , @intItemID3 int	-- (optional)??? ID
	  , @insItemCount3 smallint	-- (optional)??? ??
	  , @insItemDurability3 smallint	-- (optional)???
	  , @intRandomSeed3 int	-- (optional)?? ??
	  , @inyItemLevel3 tinyint	-- (optional)??? ??
	  , @inyItemPotential3 tinyint	-- (optional)??? ???
	  , @bitSoulBoundFlag3 bit	-- (optional)0=??, 1=?????
	  , @inySealCount3 tinyint	-- (optional)?? ???
	  , @inyItemOption3 tinyint	-- (optional)??? ??
	  , @bitEternityFlag3 bit	-- (optional)0=???? ?? ??, 1=?? ???
	  , @intItemLifespan3 int	-- (optional)??? ?? (?)
	  , @intItemID4 int	-- (optional)??? ID
	  , @insItemCount4 smallint	-- (optional)??? ??
	  , @insItemDurability4 smallint	-- (optional)???
	  , @intRandomSeed4 int	-- (optional)?? ??
	  , @inyItemLevel4 tinyint	-- (optional)??? ??
	  , @inyItemPotential4 tinyint	-- (optional)??? ???
	  , @bitSoulBoundFlag4 bit	-- (optional)0=??, 1=?????
	  , @inySealCount4 tinyint	-- (optional)?? ???
	  , @inyItemOption4 tinyint	-- (optional)??? ??
	  , @bitEternityFlag4 bit	-- (optional)0=???? ?? ??, 1=?? ???
	  , @intItemLifespan4 int	-- (optional)??? ?? (?)
	  , @bitNewFlag bit;	-- 0=?? ??? ?? ?, 1=?? ??? ?? ?


/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	
	/**_# ?? 10?? ???? ?????? Insert ???.*/
	INSERT @DonateRankers (CharacterID)
	SELECT TOP (10) MD.CharacterID
	FROM dbo.VIX_MonthlyDonation MD WITH (NOEXPAND, NOLOCK, INDEX(IX_NN_VIX_MonthlyDonation_DonationAmount_DonateDate))
	ORDER BY MD.DonationAmount DESC, DonateDate ASC;
	
	/**_# ?? ??? ??? ?????? Insert ???.*/
	INSERT @DonateRewards (RankID, RewardType, ItemID1, ItemCount1, ItemID2, ItemCount2, ItemID3, ItemCount3, ItemID4, ItemCount4)
	SELECT 1, 1, 536871410, 2, 536871411, 2, 536871415, 1, 536871414, 5
	UNION ALL
	SELECT 2, 1, 536871412, 2, 536871413, 2, 536871414, 3, NULL, NULL
	UNION ALL
	SELECT 3, 1, 536871412, 1, 536871413, 1, 536871414, 3, NULL, NULL
	UNION ALL
	SELECT 4, 1, 536871412, 1, 536871414, 3, NULL, NULL, NULL, NULL
	UNION ALL
	SELECT 5, 1, 536871413, 1, 536871414, 2, NULL, NULL, NULL, NULL
	UNION ALL
	SELECT 6, 1, 536871414, 2, NULL, NULL, NULL, NULL, NULL, NULL
	UNION ALL
	SELECT 7, 1, 536871414, 2, NULL, NULL, NULL, NULL, NULL, NULL
	UNION ALL
	SELECT 8, 1, 536871414, 1, NULL, NULL, NULL, NULL, NULL, NULL
	UNION ALL
	SELECT 9, 1, 536871414, 1, NULL, NULL, NULL, NULL, NULL, NULL
	UNION ALL
	SELECT 10, 1, 536871414, 1, NULL, NULL, NULL, NULL, NULL, NULL
	UNION ALL
	SELECT 1, 2, 536871420, 1, 536871415, 3, NULL, NULL, NULL, NULL
	UNION ALL
	SELECT 2, 2, 536871421, 1, 536871415, 3, NULL, NULL, NULL, NULL
	UNION ALL
	SELECT 3, 2, 536871422, 1, 536871415, 3, NULL, NULL, NULL, NULL
	UNION ALL
	SELECT 4, 2, 536871423, 1, 536871415, 3, NULL, NULL, NULL, NULL
	UNION ALL
	SELECT 5, 2, 536871424, 1, 536871415, 3, NULL, NULL, NULL, NULL
	UNION ALL
	SELECT 6, 2, 536871425, 1, 536871415, 1, NULL, NULL, NULL, NULL
	UNION ALL
	SELECT 7, 2, 536871425, 1, 536871415, 1, NULL, NULL, NULL, NULL
	UNION ALL
	SELECT 8, 2, 536871425, 1, 536871415, 1, NULL, NULL, NULL, NULL
	UNION ALL
	SELECT 9, 2, 536871425, 1, 536871415, 1, NULL, NULL, NULL, NULL
	UNION ALL
	SELECT 10, 2, 536871425, 1, 536871415, 1, NULL, NULL, NULL, NULL

	/**_# ???? ?? ? ?? ??? ?????? Insert ???.*/
	INSERT @DonateMailInfos (RankID, RewardType, SubjectID, ContentID)
	SELECT  1, 1, 1000040604, 1000040615
	UNION ALL
	SELECT  2, 1, 1000040605, 1000040615
	UNION ALL
	SELECT  3, 1, 1000040606, 1000040615
	UNION ALL
	SELECT  4, 1, 1000040607, 1000040615
	UNION ALL
	SELECT  5, 1, 1000040608, 1000040615
	UNION ALL
	SELECT  6, 1, 1000040609, 1000040615
	UNION ALL
	SELECT  7, 1, 1000040610, 1000040615
	UNION ALL
	SELECT  8, 1, 1000040611, 1000040615
	UNION ALL
	SELECT  9, 1, 1000040612, 1000040615
	UNION ALL
	SELECT 10, 1, 1000040613, 1000040615
	UNION ALL
	SELECT  1, 2, 1000040594, 1000040614
	UNION ALL
	SELECT  2, 2, 1000040595, 1000040614
	UNION ALL
	SELECT  3, 2, 1000040596, 1000040614
	UNION ALL
	SELECT  4, 2, 1000040597, 1000040614
	UNION ALL
	SELECT  5, 2, 1000040598, 1000040614
	UNION ALL
	SELECT  6, 2, 1000040599, 1000040614
	UNION ALL
	SELECT  7, 2, 1000040600, 1000040614
	UNION ALL
	SELECT  8, 2, 1000040601, 1000040614
	UNION ALL
	SELECT  9, 2, 1000040602, 1000040614
	UNION ALL
	SELECT 10, 2, 1000040603, 1000040614

	/**_# 1??? 10??? LOOP? ?? ?? ??? ????.*/
	WHILE @i <= 10
	BEGIN
		SELECT @inbCharacterID = CharacterID FROM @DonateRankers WHERE RankID = @i;
		IF @@ROWCOUNT = 0
			BREAK;		

		SELECT @intSubjectID = SubjectID
             , @intContentID = ContentID 
		FROM @DonateMailInfos 
		WHERE RankID = @i AND RewardType = CASE @intFirstDayFlag WHEN 1 THEN 2 ELSE 1 END			
		
		SELECT @intGiveItemID1 = ItemID1
             , @intGiveItemCount1 = ItemCount1
             , @intGiveItemID2 = ItemID2
             , @intGiveItemCount2 = ItemCount2
             , @intGiveItemID3 = ItemID3
             , @intGiveItemCount3 = ItemCount3
             , @intGiveItemID4 = ItemID4
             , @intGiveItemCount4 = ItemCount4
		FROM @DonateRewards 
		WHERE RankID = @i AND RewardType = CASE @intFirstDayFlag WHEN 1 THEN 2 ELSE 1 END

		SET @inyMailTypeCode = 1;
		SET @vchCharacterIDs = CAST(@inbCharacterID AS nvarchar(max));
		SET @dtsSendDate = NULL;
		SET @nvcSenderName = N'1000040593';--??? ??? ID? ? ? ??? ?????.
		SET @nvcSubject = CAST(@intSubjectID AS nvarchar(50));
		SET @nvcContent = CAST(@intContentID AS nvarchar(300));
		SET @inbAttachCoin = 0;
		SET @dtsMailScheduleExpireDate = NULL;
		IF @intGiveItemID1 IS NOT NULL
		BEGIN
			SET @intItemID1 = @intGiveItemID1;
			SET @insItemCount1 = @intGiveItemCount1;
			SET @insItemDurability1 = 0;
			SET @intRandomSeed1 = 0;
			SET @inyItemLevel1 = 0;
			SET @inyItemPotential1 = 0;
			SET @bitSoulBoundFlag1 = 1;
			SET @inySealCount1 = 0;
			SET @inyItemOption1 = 0;
			SET @bitEternityFlag1 = 0;
			SET @intItemLifespan1 = 0;
		END
		ELSE
		BEGIN
			SET @intItemID1 = NULL;
			SET @insItemCount1 = NULL;
			SET @insItemDurability1 = NULL;
			SET @intRandomSeed1 = NULL;
			SET @inyItemLevel1 = NULL;
			SET @inyItemPotential1 = NULL;
			SET @bitSoulBoundFlag1 = NULL;
			SET @inySealCount1 = NULL;
			SET @inyItemOption1 = NULL;
			SET @bitEternityFlag1 = NULL;
			SET @intItemLifespan1 = NULL;
		END
		IF @intGiveItemID2 IS NOT NULL
		BEGIN
			SET @intItemID2 = @intGiveItemID2;
			SET @insItemCount2 = @intGiveItemCount2;
			SET @insItemDurability2 = 0;
			SET @intRandomSeed2 = 0;
			SET @inyItemLevel2 = 0;
			SET @inyItemPotential2 = 0;
			SET @bitSoulBoundFlag2 = 1;
			SET @inySealCount2 = 0;
			SET @inyItemOption2 = 0;
			SET @bitEternityFlag2 = 0;
			SET @intItemLifespan2 = 0;
		END
		ELSE
		BEGIN
			SET @intItemID2 = NULL;
			SET @insItemCount2 = NULL;
			SET @insItemDurability2 = NULL;
			SET @intRandomSeed2 = NULL;
			SET @inyItemLevel2 = NULL;
			SET @inyItemPotential2 = NULL;
			SET @bitSoulBoundFlag2 = NULL;
			SET @inySealCount2 = NULL;
			SET @inyItemOption2 = NULL;
			SET @bitEternityFlag2 = NULL;
			SET @intItemLifespan2 = NULL;
		END
		IF @intGiveItemID3 IS NOT NULL
		BEGIN
			SET @intItemID3 = @intGiveItemID3;
			SET @insItemCount3 = @intGiveItemCount3;
			SET @insItemDurability3 = 0;
			SET @intRandomSeed3 = 0;
			SET @inyItemLevel3 = 0;
			SET @inyItemPotential3 = 0;
			SET @bitSoulBoundFlag3 = 1;
			SET @inySealCount3 = 0;
			SET @inyItemOption3 = 0;
			SET @bitEternityFlag3 = 0;
			SET @intItemLifespan3 = 0;
		END
		ELSE
		BEGIN
			SET @intItemID3 = NULL;
			SET @insItemCount3 = NULL;
			SET @insItemDurability3 = NULL;
			SET @intRandomSeed3 = NULL;
			SET @inyItemLevel3 = NULL;
			SET @inyItemPotential3 = NULL;
			SET @bitSoulBoundFlag3 = NULL;
			SET @inySealCount3 = NULL;
			SET @inyItemOption3 = NULL;
			SET @bitEternityFlag3 = NULL;
			SET @intItemLifespan3 = NULL;
		END
		IF @intGiveItemID4 IS NOT NULL
		BEGIN
			SET @intItemID4 = @intGiveItemID4;
			SET @insItemCount4 = @intGiveItemCount4;
			SET @insItemDurability4 = 0;
			SET @intRandomSeed4 = 0;
			SET @inyItemLevel4 = 0;
			SET @inyItemPotential4 = 0;
			SET @bitSoulBoundFlag4 = 1;
			SET @inySealCount4 = 0;
			SET @inyItemOption4 = 0;
			SET @bitEternityFlag4 = 0;
			SET @intItemLifespan4 = 0;
		END
		ELSE
		BEGIN
			SET @intItemID4 = NULL;
			SET @insItemCount4 = NULL;
			SET @insItemDurability4 = NULL;
			SET @intRandomSeed4 = NULL;
			SET @inyItemLevel4 = NULL;
			SET @inyItemPotential4 = NULL;
			SET @bitSoulBoundFlag4 = NULL;
			SET @inySealCount4 = NULL;
			SET @inyItemOption4 = NULL;
			SET @bitEternityFlag4 = NULL;
			SET @intItemLifespan4 = NULL;
		END
		SET @bitNewFlag = 1;
		EXEC @intReturnValue = dbo.P_AddMailSchedules
			  @inyMailTypeCode = @inyMailTypeCode
			, @vchCharacterIDs = @vchCharacterIDs
			, @dtsSendDate = @dtsSendDate
			, @nvcSenderName = @nvcSenderName
			, @nvcSubject = @nvcSubject
			, @nvcContent = @nvcContent
			, @inbAttachCoin = @inbAttachCoin
			, @dtsMailScheduleExpireDate = @dtsMailScheduleExpireDate
			, @intItemID1 = @intItemID1
			, @insItemCount1 = @insItemCount1
			, @insItemDurability1 = @insItemDurability1
			, @intRandomSeed1 = @intRandomSeed1
			, @inyItemLevel1 = @inyItemLevel1
			, @inyItemPotential1 = @inyItemPotential1
			, @bitSoulBoundFlag1 = @bitSoulBoundFlag1
			, @inySealCount1 = @inySealCount1
			, @inyItemOption1 = @inyItemOption1
			, @bitEternityFlag1 = @bitEternityFlag1
			, @intItemLifespan1 = @intItemLifespan1
			, @intItemID2 = @intItemID2
			, @insItemCount2 = @insItemCount2
			, @insItemDurability2 = @insItemDurability2
			, @intRandomSeed2 = @intRandomSeed2
			, @inyItemLevel2 = @inyItemLevel2
			, @inyItemPotential2 = @inyItemPotential2
			, @bitSoulBoundFlag2 = @bitSoulBoundFlag2
			, @inySealCount2 = @inySealCount2
			, @inyItemOption2 = @inyItemOption2
			, @bitEternityFlag2 = @bitEternityFlag2
			, @intItemLifespan2 = @intItemLifespan2
			, @intItemID3 = @intItemID3
			, @insItemCount3 = @insItemCount3
			, @insItemDurability3 = @insItemDurability3
			, @intRandomSeed3 = @intRandomSeed3
			, @inyItemLevel3 = @inyItemLevel3
			, @inyItemPotential3 = @inyItemPotential3
			, @bitSoulBoundFlag3 = @bitSoulBoundFlag3
			, @inySealCount3 = @inySealCount3
			, @inyItemOption3 = @inyItemOption3
			, @bitEternityFlag3 = @bitEternityFlag3
			, @intItemLifespan3 = @intItemLifespan3
			, @intItemID4 = @intItemID4
			, @insItemCount4 = @insItemCount4
			, @insItemDurability4 = @insItemDurability4
			, @intRandomSeed4 = @intRandomSeed4
			, @inyItemLevel4 = @inyItemLevel4
			, @inyItemPotential4 = @inyItemPotential4
			, @bitSoulBoundFlag4 = @bitSoulBoundFlag4
			, @inySealCount4 = @inySealCount4
			, @inyItemOption4 = @inyItemOption4
			, @bitEternityFlag4 = @bitEternityFlag4
			, @intItemLifespan4 = @intItemLifespan4
			, @bitNewFlag = @bitNewFlag;

		PRINT @intReturnValue;
		
		SET @i += 1;
		SET @intGiveItemID1 = NULL;
		SET @intGiveItemCount1 = NULL;
		SET @intGiveItemID2 = NULL;
		SET @intGiveItemCount2 = NULL;
		SET @intGiveItemID3 = NULL;
		SET @intGiveItemCount3 = NULL;
		SET @intGiveItemID4 = NULL;
		SET @intGiveItemCount4 = NULL;
	END
	IF @intFirstDayFlag = 1 --?? 1??? ?? ??? ????.
		/**_# ?? ? ???? Delete???. */
		DELETE dbo.DailyDonation
		WHERE DonateDate < @datBasisDate


END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
