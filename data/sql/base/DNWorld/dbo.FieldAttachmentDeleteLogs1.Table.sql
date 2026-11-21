USE [dnworld]
GO
/****** Object:  Table [dbo].[FieldAttachmentDeleteLogs1]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FieldAttachmentDeleteLogs1](
	[LogDate] [datetime] NOT NULL,
	[FarmID] [int] NOT NULL,
	[FieldIndex] [smallint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[Attachments] [xml] NOT NULL,
	[HarvestFlag] [bit] NOT NULL
) ON [DataFG1] TEXTIMAGE_ON [DataFG1]
GO
