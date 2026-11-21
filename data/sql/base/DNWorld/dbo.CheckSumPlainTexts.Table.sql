USE [dnworld]
GO
/****** Object:  Table [dbo].[CheckSumPlainTexts]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[CheckSumPlainTexts](
	[CharacterID] [bigint] NOT NULL,
	[CheckSumPlainText] [varchar](100) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[LastModifyDate] [datetime] NOT NULL,
 CONSTRAINT [PK_CheckSumPlainTexts] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
SET ANSI_PADDING OFF
GO
