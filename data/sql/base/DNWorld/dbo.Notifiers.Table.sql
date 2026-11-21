USE [dnworld]
GO
/****** Object:  Table [dbo].[Notifiers]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Notifiers](
	[CharacterID] [bigint] NOT NULL,
	[NotifierIndex] [tinyint] NOT NULL,
	[NotifyTargetCode] [tinyint] NOT NULL,
	[NotifyTargetID] [int] NOT NULL,
 CONSTRAINT [PK_Notifiers] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[NotifierIndex] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
