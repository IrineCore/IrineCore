USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDonationRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 11
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-12-23
description : ?? ?? (? & TOP 10)? ?????.
result set : CharacterID/bigint, CharacterName/nvarchar, JobCode/tinyint, GuildName/nvarchar, DonationAmount/bigint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListDonationRanking]
	  @inbCharacterID bigint --// ??? ID
	, @inbDonationAmount bigint OUTPUT --// ??? ?? ??
	, @intMyRanking int OUTPUT --// ? ?? (? ?? -1?? 10,000? ?? / -2?? ??? ?? ??? ?????.)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @dtmDonateDate datetime;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @inbDonationAmount = DonationAmount, @dtmDonateDate = DonateDate
	FROM dbo.DailyDonation WITH (NOLOCK, INDEX(PK_DailyDonation), FORCESEEK)
	WHERE CharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @inbDonationAmount = 0;
		SET @intMyRanking = -2;
	END

	ELSE
		SELECT @intMyRanking = CASE 
								WHEN SUM(CASE WHEN DonationAmount = @inbDonationAmount and DonateDate > @dtmDonateDate THEN 0 ELSE 1 END) > 10000 
									THEN -1 
								ELSE SUM(CASE WHEN DonationAmount = @inbDonationAmount and DonateDate > @dtmDonateDate THEN 0 ELSE 1 END) 
							   END
		FROM dbo.VIX_MonthlyDonation WITH (NOEXPAND, NOLOCK, INDEX(IX_NN_VIX_MonthlyDonation_DonationAmount_DonateDate), FORCESEEK)
		WHERE DonationAmount >= @inbDonationAmount
		OPTION (KEEPFIXED PLAN);

	WITH MD AS (
		SELECT TOP (10) MD.CharacterID, MD.DonationAmount
		FROM dbo.VIX_MonthlyDonation MD WITH (NOEXPAND, NOLOCK, INDEX(IX_NN_VIX_MonthlyDonation_DonationAmount_DonateDate))
		ORDER BY MD.DonationAmount DESC, DonateDate ASC
	)
	SELECT CH.CharacterID, CH.CharacterName, CHS.JobCode, G.GuildName, MD.DonationAmount
	FROM MD
		INNER LOOP JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) ON MD.CharacterID = CH.CharacterID
		INNER LOOP JOIN dbo.CharacterStatus CHS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON MD.CharacterID = CHS.CharacterID
		LEFT OUTER LOOP JOIN dbo.GuildMembers GM WITH (NOLOCK, INDEX(IX_UN_GuildMembers_CharacterID), FORCESEEK) ON MD.CharacterID = GM.CharacterID
		LEFT OUTER LOOP JOIN dbo.Guilds G WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK) ON GM.GuildID = G.GuildID
	ORDER BY MD.DonationAmount DESC;
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
