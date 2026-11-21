USE [dnmembership]
GO
/****** Object:  UserDefinedFunction [dbo].[FN_IPNumber2IP]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[FN_IPNumber2IP]') AND type in (N'FN', N'IF', N'TF', N'FS', N'FT'))
BEGIN
execute dbo.sp_executesql @statement = N'/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-04
description : ???? IP??? ?????.
**/

CREATE FUNCTION [dbo].[FN_IPNumber2IP] (
@intIPNumber int
)
RETURNS varchar(15)
AS
BEGIN
	RETURN
		CAST(((@intIPNumber & 0xFF000000) / 16777216) + 128 AS varchar(3)) + ''.'' +
		CAST((@intIPNumber & 0x00FF0000) / 65536 AS varchar(3)) + ''.'' +
		CAST((@intIPNumber & 0x0000FF00) / 256 AS varchar(3)) + ''.'' +
		CAST((@intIPNumber & 0x000000FF) AS varchar(3));
END
' 
END
GO
