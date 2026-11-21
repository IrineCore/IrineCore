USE [dnworld]
GO
/****** Object:  Table [dbo].[RankingTotalCompleteTime]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[RankingTotalCompleteTime](
	[RankingCode] [tinyint] NOT NULL,
	[CompleteDate] [datetime2](0) NOT NULL
) ON [DataFG1]
GO
