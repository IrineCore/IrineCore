USE [dnworld]
GO
/****** Object:  UserDefinedFunction [dbo].[FN_CompleteQuestBin]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-26
description : ??? ???? bitmap ???? ?????.
**/

CREATE FUNCTION [dbo].[FN_CompleteQuestBin] (
	@inbCharacterID bigint --// ??? ID
)
RETURNS binary(1250)
AS
BEGIN
	DECLARE @binCompleteQuestBin binary(1250), @vchTemp varchar(max) = '';

	WITH C1 (a, b) AS (
		SELECT QuestID / 8, CASE WHEN QuestID % 8 = 0 THEN 1 ELSE POWER(2, 8 - (QuestID % 8)) END
		FROM dbo.CompleteQuests WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID
	), C2 (a, b) AS (
		SELECT a, CONVERT(char(2), CAST(SUM(b) AS binary(1)), 2)
		FROM C1
		GROUP BY a
	)
	SELECT @vchTemp += COALESCE(C2.b, '00')
	FROM dbo.Tally T WITH (NOLOCK)
		LEFT OUTER JOIN C2 ON T.n - 1 = C2.a
	WHERE T.n <= 1250
	ORDER BY T.n;

	SET @binCompleteQuestBin = CONVERT(binary(1250), @vchTemp, 2);

	RETURN @binCompleteQuestBin;
END
GO
