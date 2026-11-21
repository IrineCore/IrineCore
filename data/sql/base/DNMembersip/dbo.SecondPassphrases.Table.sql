USE [dnmembership]
GO
/****** Object:  Table [dbo].[SecondPassphrases]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[SecondPassphrases]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[SecondPassphrases](
	[AccountID] [int] NOT NULL,
	[SecondPassphrase] [binary](32) NOT NULL,
 CONSTRAINT [PK_SecondPassphrases] PRIMARY KEY CLUSTERED 
(
	[AccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
SET ANSI_PADDING OFF
GO
