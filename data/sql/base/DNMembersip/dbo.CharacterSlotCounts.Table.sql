USE [dnmembership]
GO
/****** Object:  Table [dbo].[CharacterSlotCounts]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CharacterSlotCounts]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[CharacterSlotCounts](
	[AccountID] [int] NOT NULL,
	[WorldID] [int] NOT NULL,
	[CharacterSlotCount] [tinyint] NOT NULL,
 CONSTRAINT [PK_CharacterSlotCounts] PRIMARY KEY CLUSTERED 
(
	[AccountID] ASC,
	[WorldID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
