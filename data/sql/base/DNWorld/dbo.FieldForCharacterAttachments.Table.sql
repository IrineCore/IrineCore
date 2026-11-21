USE [dnworld]
GO
/****** Object:  Table [dbo].[FieldForCharacterAttachments]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FieldForCharacterAttachments](
	[CharacterID] [bigint] NOT NULL,
	[FieldIndex] [smallint] NOT NULL,
	[AttachedItemID] [int] NOT NULL,
	[AttachedItemCount] [int] NOT NULL,
	[AttachedDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_FieldForCharacterAttachments] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[FieldIndex] ASC,
	[AttachedItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[FieldForCharacterAttachments] ADD  CONSTRAINT [DF_FieldForCharacterAttachments_AttachedDate]  DEFAULT (getdate()) FOR [AttachedDate]
GO
